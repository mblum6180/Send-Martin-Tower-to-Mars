// menu.c - title/splash screen with fade-in logo, blinking prompt, and
// clickable "Lightly-Salted Productions" link + music toggle.
#include "game.h"
#include "assets.h"
#include "fx.h"
#include <stdio.h>
#include <math.h>

static float menu_timer, bgAlpha, titleAlpha;
static int   textBlink; // 0/1 blink for the press-start prompt

typedef struct { float x, y, w, h; } Rect;

static Rect LspButton(void) {
    return (Rect){ sys.winWidth * 0.1f, sys.winHeight * 0.9f,
                   sys.winWidth * 0.7f, sys.winHeight * 0.1f };
}
static Rect MusicButton(void) {
    return (Rect){ sys.winWidth * 0.9f, sys.winHeight * 0.8f,
                   sys.winWidth * 0.1f, sys.winHeight * 0.1f };
}

static void menu_enter(void) {
    menu_timer = 1.0f; bgAlpha = 0.0f; titleAlpha = 0.0f; textBlink = 0;
    assets.mainTheme.looping = true;
    if (GetMusicTimePlayed(assets.mainTheme) == 0.0f && !IsMusicStreamPlaying(assets.mainTheme))
        PlayMusicStream(assets.mainTheme);
}

static bool InRect(Rect r, float x, float y) {
    return x > r.x && x < r.x + r.w && y > r.y && y < r.y + r.h;
}

static float emberT;

static void menu_update(float dt) {
    menu_timer += dt;
    if (menu_timer > 1.0f) bgAlpha = FadeIn(dt, bgAlpha, 0.9f);
    if (menu_timer > 2.0f) titleAlpha = FadeIn(dt, titleAlpha, 1.0f);
    if (menu_timer > 2.0f) textBlink = (((int)menu_timer) % 2 == 0) ? 0 : 1;

    // Warm embers drifting up through the sunset for ambience.
    emberT -= dt;
    if (emberT <= 0 && bgAlpha > 0.3f) {
        Vector2 p = { (float)GetRandomValue(0, (int)sys.winWidth), sys.winHeight + 6.0f };
        FxSmoke(p, (Vector2){ (float)GetRandomValue(-12, 12), -55 }, 1,
                (Color){ 255, 170, 90, 70 }, 6, 2.2f);
        emberT = 0.06f;
    }

    // Mouse first: button clicks should not fall through to "advance".
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float mx = GetMouseX(), my = GetMouseY();
        if (InRect(LspButton(), mx, my)) { OpenURL("https://www.lightly-salted.com"); return; }
        if (InRect(MusicButton(), mx, my)) {
            if (IsMusicStreamPlaying(assets.mainTheme)) PauseMusicStream(assets.mainTheme);
            else                                        ResumeMusicStream(assets.mainTheme);
            return;
        }
    }

    // Leaderboard browser / name change. These keys are distinct from the
    // "advance" set below, so they don't fall through to starting the game.
    if (menu_timer > 2.0f) {
        if (IsKeyPressed(KEY_L)) { LeaderboardConfigure(SCREEN_MENU); ChangeScreen(SCREEN_LEADERBOARD); return; }
        if (IsKeyPressed(KEY_N)) { NameEntryConfigure(SCREEN_MENU, -1); ChangeScreen(SCREEN_NAME_ENTRY); return; }
    }

    if (menu_timer >= 3.0f) {
        bool key = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT) ||
                   IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP);
        if (key || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            SetMusicVolume(assets.mainTheme, 0.42f);
            ChangeScreen(SCREEN_INTRO02);
        }
    }
}

// Draw a texture centered at (cx, cy), scaled, with alpha.
static void DrawCentered(Texture2D t, float cx, float cy, float scale, float alpha) {
    Rectangle src = { 0, 0, (float)t.width, (float)t.height };
    Rectangle dst = { cx, cy, t.width * scale, t.height * scale };
    Vector2 origin = { t.width * scale / 2.0f, t.height * scale / 2.0f };
    DrawTexturePro(t, src, dst, origin, 0.0f, Fade(WHITE, alpha));
}

static void menu_draw(void) {
    // Background fills the window.
    DrawTexturePro(assets.titleBG,
        (Rectangle){ 0, 0, (float)assets.titleBG.width, (float)assets.titleBG.height },
        (Rectangle){ 0, 0, sys.winWidth, sys.winHeight },
        (Vector2){ 0, 0 }, 0.0f, Fade(WHITE, bgAlpha));

    float bob = sinf(menu_timer * 1.4f) * 7.0f;     // gentle title bob
    DrawCentered(assets.titleLogo, sys.winWidth * 0.5f, sys.winHeight * 0.2f + bob,
                 3.0f * sys.scaling, titleAlpha);

    Color textCol = Fade(WHITE, titleAlpha);
    DrawTextEx(assets.screenFont, "Lightly-Salted Productions 2023",
               (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.9f },
               (float)assets.screenFont.baseSize, 2.0f, textCol);
    DrawTextEx(assets.screenFont, "Music",
               (Vector2){ sys.winWidth * 0.9f, sys.winHeight * 0.8f },
               (float)assets.screenFont.baseSize, 2.0f, textCol);
    DrawWrappedText(sys.version, assets.screenFont, -20.0f, sys.winHeight * 0.9f,
                    sys.winWidth, ALIGN_RIGHT, textCol);

    if (sys.highScore > 0) {
        char hs[48];
        snprintf(hs, sizeof(hs), "Best: %d", sys.highScore);
        DrawWrappedText(hs, assets.screenFont, 0.0f, sys.winHeight * 0.82f,
                        sys.winWidth, ALIGN_CENTER, textCol);
    }

    // Leaderboard / name controls (the actual game-start prompt is the logo below).
    char hint[96];
    if (sys.playerName[0])
        snprintf(hint, sizeof(hint), "[L] Leaderboard    [N] Name: %s", sys.playerName);
    else
        snprintf(hint, sizeof(hint), "[L] Leaderboard    [N] Set Name");
    DrawWrappedText(hint, assets.screenFont, 0.0f, sys.winHeight * 0.76f,
                    sys.winWidth, ALIGN_CENTER, textCol);

    // Smoothly pulsing "press to start" (gentler than a hard blink).
    if (menu_timer > 2.0f) {
        float pulse = 0.55f + 0.45f * sinf(menu_timer * 3.2f);
        DrawCentered(assets.pressStart, sys.winWidth * 0.5f, (sys.winHeight / 3.0f) * 2.0f,
                     3.0f * sys.scaling, pulse);
    }
}

const Screen *MenuScreen(void) {
    static const Screen s = { menu_enter, menu_update, menu_draw, NULL };
    return &s;
}
