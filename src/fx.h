// fx.h - shared "juice" layer: particles, sprite explosions, and trauma-based
// screen shake. Drawn in world space (inside the shake camera) by the main loop.
#ifndef FX_H
#define FX_H

#include "raylib.h"

void FxInit(void);            // clear all pools (once at startup)
void FxUpdate(float dt);      // advance particles, explosions, shake decay
void FxDraw(void);            // draw particles + explosions (call in world space)

// Emitters --------------------------------------------------------------------
// Radial burst of `count` shrinking, fading squares.
void FxBurst(Vector2 pos, int count, Color color,
             float speedMin, float speedMax, float lifeMin, float lifeMax,
             float size, float gravity);
// Directional engine exhaust (call every frame while thrusting).
void FxExhaust(Vector2 pos, Vector2 vel, Color color, float spread, int count);
// Billowing smoke: particles that grow and fade (pass a low-alpha color).
void FxSmoke(Vector2 pos, Vector2 vel, int count, Color color, float baseSize, float life);
// Sprite explosion + debris + smoke + a big shake (used on every crash).
void FxExplosion(Vector2 pos, float scale);
// Soft dust puff (e.g. landing touchdown).
void FxDust(Vector2 pos, int count, Color color);
// Floating score text that rises and fades (e.g. "+150", "COMBO x3").
void FxPopup(Vector2 pos, const char *text, Color color, float size);

// Screen shake (trauma model: square of decaying trauma drives the offset) -----
void    FxShake(float trauma);     // add trauma in 0..1
Vector2 FxShakeOffset(void);       // current per-frame offset (feed a Camera2D)

#endif // FX_H
