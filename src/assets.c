// assets.c - load/unload all game assets once at startup.
#include "assets.h"
#include "game.h"
#include <time.h>

Assets assets;

// Anonymous Gregorian (Computus) algorithm - ported from main.lua getEasterSunday().
static void GetEasterSunday(int year, int *month, int *day) {
    int a = year % 19;
    int b = year / 100;
    int c = year % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    *month = (h + l - 7 * m + 114) / 31;
    *day   = ((h + l - 7 * m + 114) % 31) + 1;
}

static bool IsEasterToday(void) {
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    int year = now->tm_year + 1900;
    int em, ed;
    GetEasterSunday(year, &em, &ed);
    return (now->tm_mon + 1) == em && now->tm_mday == ed;
}

// Build a 4-frame horizontal strip animation (frame 107x147 in the originals).
static SpriteAnim MakeAnim(const char *path) {
    SpriteAnim a = {0};
    a.tex = LoadTexture(path);
    const float fw = 107, fh = 147;
    a.frameCount = 4;
    for (int j = 0; j < 4; j++)
        a.frames[j] = (Rectangle){ j * fw, 0, fw, fh };
    a.currentFrame = 1.0f; // 1-based like the original (Lua arrays)
    return a;
}

void LoadAssets(void) {
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);

    assets.screenFont    = LoadFontEx("assets/font.ttf", 42, NULL, 0);
    assets.messageFont   = LoadFontEx("assets/font.ttf", 80, NULL, 0);
    assets.countDownFont = LoadFontEx("assets/font.ttf", 100, NULL, 0);
    assets.scoreFont     = LoadFontEx("assets/font.ttf", 128, NULL, 0);

    assets.bgCity   = LoadTexture("assets/CityBG8bit.png");
    assets.bgSpace  = LoadTexture("assets/spaceBG.png");
    assets.bgMars   = LoadTexture("assets/marsmountain8bit.png");

    assets.titleBG    = LoadTexture("assets/title.png");
    assets.titleLogo  = LoadTexture("assets/title8bit.png");
    assets.pressStart = LoadTexture("assets/Press-space-to-start8bit.png");
    assets.icon       = LoadTexture("assets/martinIcon.png");

    assets.meteoroid = LoadTexture(IsEasterToday() ? "assets/chick8bit.png"
                                                    : "assets/asteroid.png");

    assets.fireball  = MakeAnim("assets/towerFireball.png");
    assets.explosion = MakeAnim("assets/explosion3.png");

    // Tower texture lives in the global tower struct.
    tower.image  = LoadTexture("assets/martin8bit.png");
    tower.width  = tower.image.width;
    tower.height = tower.image.height;

    assets.mainTheme = LoadMusicStream("assets/audio/Abandoned Steel Mill.ogg");
    assets.mainTheme.looping = true;
    assets.crash     = LoadSound("assets/audio/explosion-fuzzy1.ogg");
    assets.fire      = LoadSound("assets/audio/qubodupFireLoop.ogg");
    assets.launch    = LoadSound("assets/audio/rocket_launch.ogg");
    assets.itemBreak = LoadSound("assets/audio/rock_break.ogg");
    assets.marsBG    = LoadSound("assets/audio/SCAM_MIC_SOL001_RUN001.ogg");
    assets.countdown = LoadSound("assets/audio/590320main_ringtone_apollo11_countdown.ogg");
    assets.landed    = LoadSound("assets/audio/569462main_eagle_has_landed.ogg");
    assets.problem   = LoadSound("assets/audio/574928main_houston_problem.ogg");
}

void UnloadAssets(void) {
    UnloadFont(assets.screenFont);
    UnloadFont(assets.messageFont);
    UnloadFont(assets.countDownFont);
    UnloadFont(assets.scoreFont);

    UnloadTexture(assets.bgCity);
    UnloadTexture(assets.bgSpace);
    UnloadTexture(assets.bgMars);
    UnloadTexture(assets.titleBG);
    UnloadTexture(assets.titleLogo);
    UnloadTexture(assets.pressStart);
    UnloadTexture(assets.icon);
    UnloadTexture(assets.meteoroid);
    UnloadTexture(assets.fireball.tex);
    UnloadTexture(assets.explosion.tex);
    UnloadTexture(tower.image);

    UnloadMusicStream(assets.mainTheme);
    UnloadSound(assets.crash);
    UnloadSound(assets.fire);
    UnloadSound(assets.launch);
    UnloadSound(assets.itemBreak);
    UnloadSound(assets.marsBG);
    UnloadSound(assets.countdown);
    UnloadSound(assets.landed);
    UnloadSound(assets.problem);
}

void AnimAdvance(SpriteAnim *a, float amount) {
    a->currentFrame += amount;
    if (a->currentFrame >= a->frameCount) a->currentFrame = 1.0f;
}

Rectangle AnimFrame(const SpriteAnim *a) {
    int idx = (int)a->currentFrame - 1; // 1-based -> 0-based
    if (idx < 0) idx = 0;
    if (idx >= a->frameCount) idx = a->frameCount - 1;
    return a->frames[idx];
}
