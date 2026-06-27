// assets.h - one-time load/unload of all textures, fonts, audio + sprite frames.
#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

// A sprite sheet animation (replaces love newQuad frames + currentFrame).
typedef struct {
    Texture2D tex;
    Rectangle frames[4];
    int   frameCount;
    float currentFrame;   // float, advanced over time like the original
} SpriteAnim;

typedef struct {
    // Fonts (sizes match the original: 42 / 80 / 100 / 128).
    Font screenFont;
    Font messageFont;
    Font countDownFont;
    Font scoreFont;

    // Backgrounds.
    Texture2D bgCity;     // CityBG8bit.png (Level 1)
    Texture2D bgSpace;    // spaceBG.png    (Level 2)
    Texture2D bgMars;     // marsmountain8bit.png (Level 3)

    // Menu art.
    Texture2D titleBG;    // title.png
    Texture2D titleLogo;  // title8bit.png
    Texture2D pressStart; // Press-space-to-start8bit.png
    Texture2D icon;       // martinIcon.png (window icon)

    // Meteoroid sprite (asteroid, or chick on Easter).
    Texture2D meteoroid;

    // Animations.
    SpriteAnim fireball;  // towerFireball.png
    SpriteAnim explosion; // explosion3.png

    // Audio.
    Music  mainTheme;     // Abandoned Steel Mill.ogg (looped, streamed)
    Sound  crash;
    Sound  fire;
    Sound  launch;
    Sound  itemBreak;
    Sound  marsBG;
    Sound  countdown;
    Sound  landed;
    Sound  problem;
} Assets;

extern Assets assets;

void LoadAssets(void);
void UnloadAssets(void);

// Advance a sprite animation's currentFrame (wraps 1..frameCount like the original).
void AnimAdvance(SpriteAnim *a, float amount);
// Current source rectangle for drawing.
Rectangle AnimFrame(const SpriteAnim *a);

#endif // ASSETS_H
