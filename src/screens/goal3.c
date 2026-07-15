// goal3.c - final summary screen: congratulations + score breakdown, then loops.
#include "game.h"
#include "assets.h"
#include <stdio.h>

static float g3_timer, g3_bgAlpha;
static bool  g3_newBest;
static int   g3_final;     // this run's final score
static bool  g3_needName;  // true when there's no name yet to post under
static bool  g3_posted;    // an online submission was kicked off this run
static int   g3_rank;      // resolved world rank (0 = not yet / unavailable)

static void g3_enter(void) {
    g3_timer = 1.0f;
    g3_bgAlpha = 0.0f;
    g3_final = (int)sys.score03 + (int)sys.bonus;
    g3_newBest = SubmitHighScore(g3_final);  // local best (offline fallback)
    g3_rank = 0;
    g3_posted = false;
    g3_needName = false;

    // Post online when we already have a name; otherwise prompt for one (the
    // player can press [N] below to name + post). Submission is async; the rank
    // resolves once the follow-up fetch lands.
    if (sys.playerName[0]) {
        LeaderboardConfig lc = MartinLBConfig();
        const char *keys[1] = { LB_SCORE_METRIC };
        double vals[1] = { (double)g3_final };
        Leaderboard_Submit(&g_lb, &lc, sys.playerName, keys, vals, 1);
        Leaderboard_Fetch(&g_lb, &lc, keys, 1);
        g3_posted = true;
    } else {
        g3_needName = true;
    }
    sys.level += 1; // next loop is harder
}

static void g3_update(float dt) {
    g3_timer += dt;
    if (g3_timer > 1.0f) g3_bgAlpha = FadeIn(dt, g3_bgAlpha, 0.9f);

    // Resolve the world rank once the post's follow-up fetch completes.
    if (g3_posted && g3_rank == 0 && g_lb.fetch_state == LB_DONE)
        g3_rank = Leaderboard_RankFor(&g_lb, LB_SCORE_METRIC, (double)g3_final);

    // Name + post this score (only offered when no name was set).
    if (g3_needName && IsKeyPressed(KEY_N)) {
        NameEntryConfigure(SCREEN_LEADERBOARD, g3_final);
        LeaderboardConfigure(SCREEN_STAGE);
        ChangeScreen(SCREEN_NAME_ENTRY);
        return;
    }
    // View the full leaderboard, then continue into the next round.
    if (IsKeyPressed(KEY_L)) {
        LeaderboardConfigure(SCREEN_STAGE);
        ChangeScreen(SCREEN_LEADERBOARD);
        return;
    }

    if (g3_timer >= 3.0f) {
        bool key = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT) ||
                   IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP);
        if (key || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetMusicTarget(0.42f);
            ChangeScreen(SCREEN_STAGE);
        }
    }
}

static void g3_draw(void) {
    DrawStarfield();
    Color c = Fade(WHITE, g3_bgAlpha);
    DrawWrappedText(
        "Congratulations!\n\n"
        "Martin Tower has safely landed on the red planet, Mars. Your expert navigation "
        "skills and quick reflexes made this historic mission a success.\n\nWell done!",
        assets.screenFont, sys.winWidth * 0.1f, sys.winHeight * 0.05f,
        sys.winWidth * 0.8f, ALIGN_CENTER, c);

    char buf[256];
    int final = (int)sys.score03 + (int)sys.bonus;
    snprintf(buf, sizeof(buf),
             "Meteoroids hit: %d\nFuel: %d\nBonus: %d\nFinal Score: %d",
             sys.itemsDestroyed, (int)sys.score03, (int)sys.bonus, final);
    DrawWrappedText(buf, assets.screenFont, sys.winWidth * 0.1f, sys.winHeight * 0.585f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, c);

    // One result line: the local best plus an online tag (world rank when it
    // resolves, posting/offline state, or a prompt to name + post the score).
    Color cyan = Fade((Color){ 90, 230, 200, 255 }, g3_bgAlpha);
    const char *bestLbl = g3_newBest ? "NEW BEST!" : "Best";
    if (g3_needName)
        snprintf(buf, sizeof(buf), "%s: %d      [N] Post score", bestLbl, sys.highScore);
    else if (g3_posted && g3_rank > 0)
        snprintf(buf, sizeof(buf), "%s: %d      World Rank #%d", bestLbl, sys.highScore, g3_rank);
    else if (g3_posted && g_lb.net_status == LB_NET_OFFLINE)
        snprintf(buf, sizeof(buf), "%s: %d      (offline)", bestLbl, sys.highScore);
    else if (g3_posted)
        snprintf(buf, sizeof(buf), "%s: %d      (posting...)", bestLbl, sys.highScore);
    else
        snprintf(buf, sizeof(buf), "%s: %d", bestLbl, sys.highScore);
    DrawWrappedText(buf, assets.screenFont, sys.winWidth * 0.1f, sys.winHeight * 0.855f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, g3_newBest ? cyan : c);

    DrawWrappedText("Press to restart      [L] Leaderboard", assets.screenFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.925f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, c);
}

static void g3_leave(void) {
    ResetGame();
}

const Screen *Goal3Screen(void) {
    static const Screen s = { g3_enter, g3_update, g3_draw, g3_leave };
    return &s;
}
