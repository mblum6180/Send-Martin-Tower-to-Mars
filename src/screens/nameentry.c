// nameentry.c - keyboard entry of the leaderboard display name.
//
// Reached from the menu ("change name") and from the Mars-landing summary when a
// score is ready to post but no name is set yet. On confirm the name is persisted
// and, if a score was queued via NameEntryConfigure(), submitted online. When a
// leaderboard is configured and we're not known-offline, the name is first
// pre-flighted against the server's moderation filter so a rejected name surfaces
// here instead of silently failing to post.
#include "game.h"
#include "assets.h"
#include <string.h>
#include <stdio.h>

#define NE_MAX_CHARS 16

static char       ne_buf[NE_MAX_CHARS + 1];
static int        ne_len;
static GameScreen ne_return;       // where to go on confirm/cancel
static int        ne_submit;       // score to post after accept, or < 0 for none
static bool       ne_validating;   // waiting on a server name pre-flight
static char       ne_error[48];
static float      ne_timer;        // drives the caret blink

// Server charset (mirrors the leaderboard server's NAME_RE: [A-Za-z0-9_ .'-]).
// Disallowed keystrokes are dropped at entry time so a confirmed name always
// passes server validation.
static bool ne_char_ok(int ch) {
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch >= 'a' && ch <= 'z') return true;
    if (ch >= '0' && ch <= '9') return true;
    return ch == '_' || ch == ' ' || ch == '.' || ch == '\'' || ch == '-';
}

void NameEntryConfigure(GameScreen returnScreen, int submitScore) {
    ne_return = returnScreen;
    ne_submit = submitScore;
}

static void ne_enter(void) {
    // Seed from the saved name, truncated to the editable width.
    strncpy(ne_buf, sys.playerName, NE_MAX_CHARS);
    ne_buf[NE_MAX_CHARS] = '\0';
    ne_len = (int)strlen(ne_buf);
    ne_validating = false;
    ne_error[0] = '\0';
    ne_timer = 0.0f;
}

// Accept the typed name: persist it and (if queued) post the score + refresh.
static void ne_finish(void) {
    snprintf(sys.playerName, sizeof(sys.playerName), "%s", ne_buf);
    SavePlayerName();
    if (ne_submit >= 0) {
        LeaderboardConfig lc = MartinLBConfig();
        const char *keys[1] = { LB_SCORE_METRIC };
        double vals[1] = { (double)ne_submit };
        Leaderboard_Submit(&g_lb, &lc, sys.playerName, keys, vals, 1);
        Leaderboard_Fetch(&g_lb, &lc, keys, 1);
    }
    ChangeScreen(ne_return);
}

static void ne_confirm(void) {
    if (ne_len == 0) return;  // require at least one character
    ne_error[0] = '\0';
    LeaderboardConfig lc = MartinLBConfig();
    // Pre-flight unless we KNOW we're offline (treat not-yet-probed as "try it";
    // it fails open fast if we do turn out to be down).
    if (lc.url[0] && !Leaderboard_IsOffline(&g_lb)) {
        Leaderboard_ValidateName(&g_lb, &lc, ne_buf);
        ne_validating = true;
    } else {
        ne_finish();
    }
}

static void ne_update(float dt) {
    ne_timer += dt;

    // While a name pre-flight is in flight, swallow input and wait for the result.
    if (ne_validating) {
        bool blocked = false;
        if (Leaderboard_ValidatePollDone(&g_lb, &blocked)) {
            ne_validating = false;
            if (blocked) snprintf(ne_error, sizeof(ne_error), "NAME UNAVAILABLE - PICK ANOTHER");
            else         ne_finish();
        }
        return;
    }

    int c;
    while ((c = GetCharPressed()) != 0) {
        if (ne_len < NE_MAX_CHARS && ne_char_ok(c)) {
            ne_buf[ne_len++] = (char)c;
            ne_buf[ne_len] = '\0';
            ne_error[0] = '\0';
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE) && ne_len > 0) {
        ne_buf[--ne_len] = '\0';
        ne_error[0] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) ne_confirm();
}

static void ne_draw(void) {
    DrawStarfield();

    DrawWrappedText("ENTER YOUR NAME", assets.messageFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.18f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, WHITE);

    // The entered text with a blinking caret while not validating.
    char shown[NE_MAX_CHARS + 2];
    bool caret = !ne_validating && (((int)(ne_timer * 2.0f)) % 2 == 0);
    snprintf(shown, sizeof(shown), "%s%s", ne_buf, caret ? "_" : "");
    DrawWrappedText(shown[0] ? shown : " ", assets.scoreFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.40f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, (Color){ 90, 230, 200, 255 });

    if (ne_validating) {
        DrawWrappedText("CHECKING...", assets.screenFont,
                        sys.winWidth * 0.1f, sys.winHeight * 0.70f,
                        sys.winWidth * 0.8f, ALIGN_CENTER, (Color){ 200, 200, 200, 255 });
    } else if (ne_error[0]) {
        DrawWrappedText(ne_error, assets.screenFont,
                        sys.winWidth * 0.1f, sys.winHeight * 0.70f,
                        sys.winWidth * 0.8f, ALIGN_CENTER, (Color){ 255, 90, 90, 255 });
    }

    DrawWrappedText("Type a name, then press ENTER", assets.screenFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.85f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, (Color){ 200, 200, 200, 255 });
}

const Screen *NameEntryScreen(void) {
    static const Screen s = { ne_enter, ne_update, ne_draw, NULL };
    return &s;
}
