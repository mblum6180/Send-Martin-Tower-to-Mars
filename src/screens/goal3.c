// goal3.c - final summary screen: congratulations + score breakdown, then loops.
#include "game.h"
#include "assets.h"
#include <stdio.h>

static float g3_timer, g3_bgAlpha;
static bool  g3_newBest;

static void g3_enter(void) {
    g3_timer = 1.0f;
    g3_bgAlpha = 0.0f;
    g3_newBest = SubmitHighScore((int)sys.score03 + (int)sys.bonus);
    sys.level += 1; // next loop is harder
}

static void g3_update(float dt) {
    g3_timer += dt;
    if (g3_timer > 1.0f) g3_bgAlpha = FadeIn(dt, g3_bgAlpha, 0.9f);
    if (g3_timer >= 3.0f) {
        bool key = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT) ||
                   IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP);
        if (key || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetMusicVolume(assets.mainTheme, 0.42f);
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

    snprintf(buf, sizeof(buf), "%s: %d", g3_newBest ? "NEW BEST!" : "Best", sys.highScore);
    DrawWrappedText(buf, assets.screenFont, sys.winWidth * 0.1f, sys.winHeight * 0.86f,
                    sys.winWidth * 0.8f, ALIGN_CENTER,
                    g3_newBest ? Fade((Color){ 90, 230, 200, 255 }, g3_bgAlpha) : c);

    DrawWrappedText("Press to restart", assets.screenFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.93f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, c);
}

static void g3_leave(void) {
    ResetGame();
}

const Screen *Goal3Screen(void) {
    static const Screen s = { g3_enter, g3_update, g3_draw, g3_leave };
    return &s;
}
