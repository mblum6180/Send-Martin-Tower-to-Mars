// leaderboard.c - worldwide top-10 high-score list.
//
// Fetches the "score" board on enter and renders it over the starfield. Falls
// back gracefully to LOADING / OFFLINE / NO CONNECTION states so the screen is
// safe to open even with no server reachable. Any key or click returns to the
// configured caller screen (menu by default).
#include "game.h"
#include "assets.h"
#include <stdio.h>
#include <math.h>

static GameScreen lb_return = SCREEN_MENU;
static float      lb_timer;

void LeaderboardConfigure(GameScreen returnScreen) { lb_return = returnScreen; }

static void lb_enter(void) {
    lb_timer = 0.0f;
    LeaderboardConfig lc = MartinLBConfig();
    const char *metrics[1] = { LB_SCORE_METRIC };
    Leaderboard_Fetch(&g_lb, &lc, metrics, 1);
}

static void lb_update(float dt) {
    lb_timer += dt;
    bool key = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
               IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) ||
               IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP) ||
               IsKeyPressed(KEY_BACKSPACE);
    if (key || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ChangeScreen(lb_return);
}

static void lb_center(const char *text, Font font, float y, Color col) {
    DrawWrappedText(text, font, sys.winWidth * 0.1f, y,
                    sys.winWidth * 0.8f, ALIGN_CENTER, col);
}

static void lb_draw(void) {
    DrawStarfield();

    lb_center("HIGH SCORES", assets.messageFont, sys.winHeight * 0.06f,
              (Color){ 255, 215, 70, 255 });

    char best[64];
    snprintf(best, sizeof(best), "YOUR BEST: %d", sys.highScore);
    lb_center(best, assets.screenFont, sys.winHeight * 0.24f, (Color){ 180, 180, 180, 255 });

    const LBBoard *board = Leaderboard_FindBoard(&g_lb, LB_SCORE_METRIC);

    if (g_lb.fetch_state == LB_PENDING) {
        lb_center("LOADING...", assets.screenFont, sys.winHeight * 0.5f, WHITE);
    } else if (g_lb.fetch_state == LB_ERROR || !board || board->count == 0) {
        const char *msg = (g_lb.net_status == LB_NET_OFFLINE)
                        ? "OFFLINE - LEADERBOARD DISABLED"
                        : "NO CONNECTION";
        lb_center(msg, assets.screenFont, sys.winHeight * 0.5f, (Color){ 160, 160, 160, 255 });
    } else {
        float x   = sys.winWidth * 0.22f;
        float xs  = sys.winWidth * 0.78f;       // right edge for the score column
        float y   = sys.winHeight * 0.34f;
        float rh  = sys.winHeight * 0.055f;
        float fs  = (float)assets.screenFont.baseSize;
        for (int i = 0; i < board->count; i++) {
            const LBEntry *e = &board->entries[i];
            char rank[48];
            snprintf(rank, sizeof(rank), "%2d. %s", i + 1, e->name);
            DrawTextEx(assets.screenFont, rank, (Vector2){ x, y }, fs, 2.0f, WHITE);

            char val[24];
            snprintf(val, sizeof(val), "%d", (int)LBEntry_Get(e, LB_SCORE_METRIC, 0));
            Vector2 vm = MeasureTextEx(assets.screenFont, val, fs, 2.0f);
            DrawTextEx(assets.screenFont, val, (Vector2){ xs - vm.x, y }, fs, 2.0f,
                       (Color){ 90, 230, 200, 255 });
            y += rh;
        }
    }

    // Gentle blink on the back prompt.
    float a = 0.55f + 0.45f * sinf(lb_timer * 3.2f);
    lb_center("Press any key to go back", assets.screenFont, sys.winHeight * 0.9f,
              Fade(WHITE, a));
}

const Screen *LeaderboardScreen(void) {
    static const Screen s = { lb_enter, lb_update, lb_draw, NULL };
    return &s;
}
