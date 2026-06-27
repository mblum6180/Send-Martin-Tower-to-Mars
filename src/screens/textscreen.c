// textscreen.c - shared driver for the simple fade-in text screens
// (intro, intro02, goal1, goal2). Each is identical apart from its body text
// and the screen it advances to.
#include "game.h"
#include "assets.h"

static float       ts_timer;
static float       ts_bgAlpha;
static const char *ts_body;
static GameScreen  ts_next;

static void ts_begin(const char *body, GameScreen next) {
    ts_body = body; ts_next = next;
    ts_timer = 1.0f; ts_bgAlpha = 0.0f;
}

static void ts_update(float dt) {
    ts_timer += dt;
    if (ts_timer > 1.0f) ts_bgAlpha = FadeIn(dt, ts_bgAlpha, 0.9f);

    if (ts_timer >= 3.0f) {
        bool key = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT) ||
                   IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP);
        bool tap = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        if (key || tap) {
            SetMusicVolume(assets.mainTheme, 0.42f);
            ChangeScreen(ts_next);
        }
    }
}

static void ts_draw(void) {
    DrawStarfield();
    DrawWrappedText(ts_body, assets.screenFont,
                    sys.winWidth * 0.1f, sys.winHeight * 0.1f,
                    sys.winWidth * 0.8f, ALIGN_CENTER, Fade(WHITE, ts_bgAlpha));
}

// --- intro ------------------------------------------------------------------
static void intro_enter(void) {
    ts_begin("Martin Tower, once a Bethlehem landmark, is now a symbol of human "
             "determination and technological advancement.\n\n"
             "As a rocket ship, it will embark on a journey to Mars to serve as the "
             "foundation for a new colony.\n\n"
             "Martin Tower will stand as a testament to our unrelenting spirit of "
             "exploration and innovation. The world awaits its historic journey.",
             SCREEN_MENU);
}
const Screen *IntroScreen(void) {
    static const Screen s = { intro_enter, ts_update, ts_draw, NULL };
    return &s;
}

// --- intro02 ----------------------------------------------------------------
static void intro02_enter(void) {
    ts_begin("Get ready to blast off to Mars with Martin Tower!\n\n"
             "Fill up the fuel tank for launch by rapidly pressing the Left and Right keys.\n\n"
             "Set off on your mission to Mars now!",
             SCREEN_LEVEL1);
}
const Screen *Intro02Screen(void) {
    static const Screen s = { intro02_enter, ts_update, ts_draw, NULL };
    return &s;
}

// --- goal1 ------------------------------------------------------------------
static void goal1_enter(void) {
    ts_begin("Martin Tower is flying through the vast emptiness of space on its way to Mars!\n"
             "But beware, navigating the treacherous meteoroid field will test your skills.\n\n"
             "If Martin Tower collides with a meteoroid, precious fuel will be lost.\n\n"
             "Are you ready to tackle this challenge?",
             SCREEN_LEVEL2);
}
const Screen *Goal1Screen(void) {
    static const Screen s = { goal1_enter, ts_update, ts_draw, NULL };
    return &s;
}

// --- goal2 ------------------------------------------------------------------
static void goal2_enter(void) {
    ts_begin("Well done! You navigated the dangerous meteoroid field with ease.\n\n"
             "Next, it's time to softly land Martin Tower at its new home on the red planet, Mars.\n\n"
             "Press center or up to slow your descent.",
             SCREEN_LEVEL3);
}
const Screen *Goal2Screen(void) {
    static const Screen s = { goal2_enter, ts_update, ts_draw, NULL };
    return &s;
}
