// fx.c - particles, sprite explosions, floating popups, and screen shake.
#include "fx.h"
#include "assets.h"
#include <math.h>
#include <string.h>

#define FX_MAX_PARTICLES 1024
#define FX_MAX_EXPLO     24
#define FX_MAX_POPUPS    40

typedef struct {
    Vector2 pos, vel;
    float life, maxLife, size, growth, gravity, drag;
    Color color;            // .a is the particle's peak alpha
    bool  active;
} Particle;

typedef struct { Vector2 pos; float scale, frame; bool active; } Explosion;
typedef struct {
    Vector2 pos, vel;
    float life, maxLife, size;
    Color color;
    char  text[20];
    bool  active;
} TextPop;

static Particle  parts[FX_MAX_PARTICLES];
static Explosion explos[FX_MAX_EXPLO];
static TextPop   pops[FX_MAX_POPUPS];
static float     trauma;
static int       nextPart;   // round-robin cursor (cheap allocation)

static float Frnd(float a, float b) {
    return a + (float)GetRandomValue(0, 10000) / 10000.0f * (b - a);
}

void FxInit(void) {
    for (int i = 0; i < FX_MAX_PARTICLES; i++) parts[i].active = false;
    for (int i = 0; i < FX_MAX_EXPLO; i++) explos[i].active = false;
    for (int i = 0; i < FX_MAX_POPUPS; i++) pops[i].active = false;
    trauma = 0;
    nextPart = 0;
}

void FxPopup(Vector2 pos, const char *text, Color color, float size) {
    for (int i = 0; i < FX_MAX_POPUPS; i++) {
        if (pops[i].active) continue;
        pops[i].pos = pos;
        pops[i].vel = (Vector2){ Frnd(-12, 12), -70 };
        pops[i].maxLife = pops[i].life = 0.95f;
        pops[i].size = size;
        pops[i].color = color;
        strncpy(pops[i].text, text, sizeof(pops[i].text) - 1);
        pops[i].text[sizeof(pops[i].text) - 1] = 0;
        pops[i].active = true;
        return;
    }
}

// Grab a slot, overwriting the oldest if the pool is saturated (bursts are fine
// to clip slightly; round-robin keeps it cheap and visually even).
static Particle *Alloc(void) {
    for (int i = 0; i < FX_MAX_PARTICLES; i++) {
        Particle *p = &parts[nextPart];
        nextPart = (nextPart + 1) % FX_MAX_PARTICLES;
        if (!p->active) return p;
    }
    Particle *p = &parts[nextPart];
    nextPart = (nextPart + 1) % FX_MAX_PARTICLES;
    return p;
}

void FxBurst(Vector2 pos, int count, Color color, float spMin, float spMax,
             float lMin, float lMax, float size, float gravity) {
    for (int i = 0; i < count; i++) {
        Particle *p = Alloc();
        float ang = Frnd(0, 6.28318f), sp = Frnd(spMin, spMax);
        p->pos = pos;
        p->vel = (Vector2){ cosf(ang) * sp, sinf(ang) * sp };
        p->maxLife = p->life = Frnd(lMin, lMax);
        p->size = size * Frnd(0.6f, 1.3f);
        p->growth = -p->size / p->maxLife;     // shrink to nothing
        p->gravity = gravity;
        p->drag = 0.9f;
        p->color = color;
        p->active = true;
    }
}

void FxExhaust(Vector2 pos, Vector2 vel, Color color, float spread, int count) {
    for (int i = 0; i < count; i++) {
        Particle *p = Alloc();
        p->pos = (Vector2){ pos.x + Frnd(-spread, spread), pos.y + Frnd(-spread, spread) };
        p->vel = (Vector2){ vel.x + Frnd(-40, 40), vel.y + Frnd(-40, 40) };
        p->maxLife = p->life = Frnd(0.25f, 0.55f);
        p->size = Frnd(4, 8);
        p->growth = -p->size / p->maxLife;
        p->gravity = 0;
        p->drag = 2.2f;
        p->color = color;
        p->active = true;
    }
}

void FxSmoke(Vector2 pos, Vector2 vel, int count, Color color, float baseSize, float life) {
    for (int i = 0; i < count; i++) {
        Particle *p = Alloc();
        p->pos = (Vector2){ pos.x + Frnd(-6, 6), pos.y + Frnd(-6, 6) };
        p->vel = (Vector2){ vel.x + Frnd(-30, 30), vel.y + Frnd(-30, 30) };
        p->maxLife = p->life = life * Frnd(0.7f, 1.2f);
        p->size = baseSize * Frnd(0.7f, 1.2f);
        p->growth = p->size * 1.6f;            // billow outward as it rises
        p->gravity = 0;
        p->drag = 1.4f;
        p->color = color;
        p->active = true;
    }
}

void FxDust(Vector2 pos, int count, Color color) {
    for (int i = 0; i < count; i++) {
        Particle *p = Alloc();
        float ang = Frnd(3.6f, 5.8f);            // upward-ish fan
        float sp = Frnd(40, 180);
        p->pos = pos;
        p->vel = (Vector2){ cosf(ang) * sp, sinf(ang) * sp };
        p->maxLife = p->life = Frnd(0.4f, 0.9f);
        p->size = Frnd(4, 9);
        p->growth = p->size * 0.4f;              // puff out a little
        p->gravity = 200;
        p->drag = 1.2f;
        p->color = color;
        p->active = true;
    }
}

void FxExplosion(Vector2 pos, float scale) {
    for (int i = 0; i < FX_MAX_EXPLO; i++)
        if (!explos[i].active) { explos[i] = (Explosion){ pos, scale, 1.0f, true }; break; }
    FxBurst(pos, 70, (Color){ 255, 200, 70, 255 }, 90, 520, 0.4f, 1.0f, 6, 360);
    FxBurst(pos, 34, (Color){ 255, 90, 40, 255 }, 40, 300, 0.3f, 0.8f, 9, 140);
    FxBurst(pos, 26, (Color){ 110, 80, 64, 255 }, 50, 280, 0.5f, 1.2f, 7, 420);
    FxSmoke(pos, (Vector2){ 0, -40 }, 22, (Color){ 60, 55, 55, 150 }, 16 * scale, 1.1f);
    FxShake(0.8f);
}

void    FxShake(float t) { trauma += t; if (trauma > 1.0f) trauma = 1.0f; }

Vector2 FxShakeOffset(void) {
    if (trauma <= 0) return (Vector2){ 0, 0 };
    float s = trauma * trauma, mag = 16.0f * s;
    return (Vector2){ Frnd(-1, 1) * mag, Frnd(-1, 1) * mag };
}

void FxUpdate(float dt) {
    if (trauma > 0) { trauma -= dt * 1.8f; if (trauma < 0) trauma = 0; }

    for (int i = 0; i < FX_MAX_PARTICLES; i++) {
        Particle *p = &parts[i];
        if (!p->active) continue;
        p->life -= dt;
        if (p->life <= 0) { p->active = false; continue; }
        float damp = 1.0f - p->drag * dt; if (damp < 0) damp = 0;
        p->vel.x *= damp; p->vel.y *= damp;
        p->vel.y += p->gravity * dt;
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->size += p->growth * dt;
        if (p->size < 0) p->size = 0;
    }

    for (int i = 0; i < FX_MAX_EXPLO; i++) {
        Explosion *e = &explos[i];
        if (!e->active) continue;
        e->frame += dt * 16.0f;
        if (e->frame >= assets.explosion.frameCount + 1) e->active = false;
    }

    for (int i = 0; i < FX_MAX_POPUPS; i++) {
        TextPop *q = &pops[i];
        if (!q->active) continue;
        q->life -= dt;
        if (q->life <= 0) { q->active = false; continue; }
        q->vel.y += 40.0f * dt;          // ease the rise to a stop
        q->pos.x += q->vel.x * dt;
        q->pos.y += q->vel.y * dt;
    }
}

void FxDraw(void) {
    for (int i = 0; i < FX_MAX_PARTICLES; i++) {
        Particle *p = &parts[i];
        if (!p->active) continue;
        float t = p->life / p->maxLife;          // 1 -> 0
        if (t > 1) t = 1;
        Color c = p->color;
        c.a = (unsigned char)(p->color.a * t);   // fade from the particle's peak alpha
        float s = p->size;
        DrawRectangle((int)(p->pos.x - s * 0.5f), (int)(p->pos.y - s * 0.5f),
                      (int)(s + 1), (int)(s + 1), c);
    }
    for (int i = 0; i < FX_MAX_EXPLO; i++) {
        Explosion *e = &explos[i];
        if (!e->active) continue;
        int idx = (int)e->frame - 1;
        if (idx < 0) idx = 0;
        if (idx >= assets.explosion.frameCount) idx = assets.explosion.frameCount - 1;
        float w = 107 * e->scale, h = 147 * e->scale;
        DrawTexturePro(assets.explosion.tex, assets.explosion.frames[idx],
                       (Rectangle){ e->pos.x, e->pos.y, w, h },
                       (Vector2){ w * 0.5f, h * 0.5f }, 0.0f, WHITE);
    }

    for (int i = 0; i < FX_MAX_POPUPS; i++) {
        TextPop *q = &pops[i];
        if (!q->active) continue;
        float t = q->life / q->maxLife;
        unsigned char a = (unsigned char)(255.0f * (t > 1 ? 1 : t));
        Vector2 m = MeasureTextEx(assets.screenFont, q->text, q->size, 1);
        Vector2 at = { q->pos.x - m.x * 0.5f, q->pos.y - m.y * 0.5f };
        DrawTextEx(assets.screenFont, q->text, (Vector2){ at.x + 2, at.y + 2 },
                   q->size, 1, (Color){ 0, 0, 0, (unsigned char)(a * 0.6f) });   // shadow
        Color c = q->color; c.a = a;
        DrawTextEx(assets.screenFont, q->text, at, q->size, 1, c);
    }
}
