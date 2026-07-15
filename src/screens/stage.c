// stage.c - "Round: N" interstitial shown before each loop's first level.
#include "game.h"
#include "assets.h"
#include <stdio.h>

static float stage_timer, stage_bgAlpha;

static void stage_enter(void) {
    stage_timer = 1.0f;
    stage_bgAlpha = 0.0f;
}

static void stage_update(float dt) {
    stage_timer += dt;
    if (stage_timer > 1.0f) stage_bgAlpha = FadeIn(dt, stage_bgAlpha, 0.9f);
    if (stage_timer >= 3.0f) {
        bool key = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT) ||
                   IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP);
        if (key || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetMusicTarget(0.42f);
            ChangeScreen(SCREEN_LEVEL1);
        }
    }
}

static void stage_draw(void) {
    DrawStarfield();
    char buf[64];
    snprintf(buf, sizeof(buf), "Round: %d", sys.level);
    DrawWrappedText(buf, assets.scoreFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.3f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, Fade(WHITE, stage_bgAlpha));
}

const Screen *StageScreen(void) {
    static const Screen s = { stage_enter, stage_update, stage_draw, NULL };
    return &s;
}
