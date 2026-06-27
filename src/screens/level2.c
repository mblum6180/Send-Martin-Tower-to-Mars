// level2.c - Space: vertical auto-scroll, dodge spinning meteoroids; collisions
// and time cost fuel. Ported from gameLevel02.lua.
//
// The original used Box2D, but this level is an on-rails scroller (the tower's
// Y is locked and only its X is steered), so it's implemented with lightweight
// manual motion + manual circle-vs-tower collision (as the plan anticipated).
// physac is reserved for L1/L3 where gravity/torque/restitution matter.
#include "game.h"
#include "assets.h"
#include "input.h"
#include "fx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_METEOROIDS 64
#define NSTARS 140

// Parallax starfield (depth behind the meteoroids).
typedef struct { float x, y, size, bright, par; } Star;
static Star stars[NSTARS];

typedef struct {
    bool  active;
    float wx, wy;        // world position (screenY = wy + scroll)
    float vx, vy;        // drift
    float angle, angVel; // spin
    float scale;         // draw/collision scale (asteroid is 32px)
    float red, grn, blu; // tint; red drops 0.5 per hit, destroyed at <=0
    bool  touching;      // currently overlapping tower (edge-detect contacts)
    float gap;           // closest approach this pass (edge gap; <0 = overlapped)
    bool  grazeCounted;  // graze already scored when it passed the tower
} Meteoroid;

static Meteoroid junk[MAX_METEOROIDS];
static float scroll, scrollTower, junkTimer, junkInterval;
static float towerX, towerVX;
static float grazeFlash;          // seconds remaining on the "GRAZE!" popup
static int   grazeCount;          // grazes this run
static int   combo;               // consecutive grazes (resets on a hit)
#define MAX_COMBO 5

// --- tuning -----------------------------------------------------------------
// Steering is a heavy "rocket" feel: gentle sideways thrust + low damping so the
// tower carries momentum and is slow to start/stop (not twitchy).
static float L2_SCROLL = 42.0f;   // world scroll speed (px/s)
static float L2_SIDE   = 820.0f;  // sideways thrust accel (px/s^2)
static float L2_DAMP   = 1.8f;    // velocity damping (per s) - low = more inertia
static float L2_SPAWN  = 2.5f;    // per-level spawn-rate factor (asteroid density)
static float L2_GRAZE  = 48.0f;   // edge gap (px) that counts as a near miss
#define GRAZE_BONUS  15           // fuel awarded per graze
static float EnvF(const char *k, float def){ const char*v=getenv(k); return v?(float)atof(v):def; }

static float Frnd(float a, float b) { return a + (float)GetRandomValue(0, 10000)/10000.0f * (b - a); }
static const float TOWER_W = 79.0f, TOWER_H = 128.0f; // native sprite size (scale 1 in L2)

static void NewInterval(void) { junkInterval = Frnd(0.3f, 4.0f); }

static void l2_enter(void) {
    L2_SCROLL = EnvF("L2_SCROLL", 42.0f);
    L2_SIDE   = EnvF("L2_SIDE", 820.0f);
    L2_DAMP   = EnvF("L2_DAMP", 1.8f);
    L2_SPAWN  = EnvF("L2_SPAWN", 2.5f);
    L2_GRAZE  = EnvF("L2_GRAZE", 48.0f);
    for (int i = 0; i < MAX_METEOROIDS; i++) junk[i].active = false;
    scroll = 0; scrollTower = 0; junkTimer = 0; NewInterval();
    grazeFlash = 0; grazeCount = 0; combo = 0;
    for (int i = 0; i < NSTARS; i++) {
        stars[i].x = Frnd(0, sys.winWidth);
        stars[i].y = Frnd(0, sys.winHeight);
        stars[i].size = Frnd(1, 3);
        stars[i].bright = Frnd(0.3f, 1.0f);
        stars[i].par = Frnd(0.08f, 0.5f);   // closer stars (bigger par) scroll faster
    }
    sys.score02 = sys.score01;
    sys.crashed = false;
    sys.BGScale = 1.0f;
    sys.BGcolorR = sys.BGcolorG = sys.BGcolorB = 1.0f;
    towerX = Frnd(sys.winWidth * 0.2f, sys.winWidth * 0.8f);
    towerVX = 0;

    SetSoundVolume(assets.fire, 0.16f);
    PlaySound(assets.fire);
}

static void Spawn(void) {
    int slot = -1;
    for (int i = 0; i < MAX_METEOROIDS; i++) if (!junk[i].active) { slot = i; break; }
    if (slot < 0) return;
    Meteoroid *m = &junk[slot];
    m->active = true;
    float scale = Frnd(0.8f, 1.5f);
    if (scale >= 1.45f) scale = Frnd(1.4f, 3.2f);
    m->scale = scale;
    m->wx = Frnd(0, sys.winWidth);
    m->wy = -scroll - 32.0f * scale;            // just above the top of view
    m->vx = Frnd(-40, 40);
    m->vy = Frnd(-10, 30);
    m->angle = Frnd(0, 6.283f);
    m->angVel = Frnd(-1.2f, 1.2f);
    m->red = 1.0f; m->grn = Frnd(0.6f, 1.0f); m->blu = Frnd(0.8f, 1.0f);
    m->touching = false;
    m->gap = 1e9f; m->grazeCounted = false;
}

// Gap between a circle (cx,cy,r) and the tower rect: distance from the circle's
// edge to the nearest point on the rect (<0 = overlapping = a hit).
static float TowerGap(float cx, float cy, float r, float rx, float ry) {
    float nx = cx < rx ? rx : (cx > rx + TOWER_W ? rx + TOWER_W : cx);
    float ny = cy < ry ? ry : (cy > ry + TOWER_H ? ry + TOWER_H : cy);
    float dx = cx - nx, dy = cy - ny;
    return sqrtf(dx * dx + dy * dy) - r;
}

static void l2_update(float dt) {
    AnimAdvance(&assets.fireball, 10.0f * dt);
    if (!IsSoundPlaying(assets.fire) && !sys.crashed) PlaySound(assets.fire); // loop

    // Spawn cadence scales with difficulty (level); L2_SPAWN overrides the rate.
    junkTimer += sys.scaling * dt * (sys.level * L2_SPAWN);
    if (junkTimer > junkInterval) { Spawn(); junkTimer = 0; NewInterval(); }

    float towerScreenY = sys.winHeight * 0.95f - scrollTower;

    // Centered engine exhaust trailing below the rocket.
    if (!sys.crashed) {
        Vector2 ex = { towerX + TOWER_W * 0.5f, towerScreenY + TOWER_H * 0.98f };
        FxExhaust(ex, (Vector2){ 0, 170 }, (Color){ 255, 170, 50, 255 }, 9, 2);
    }

    // Steering: hold Left/Right (or the screen edges) for continuous, gentle
    // sideways thrust. Low damping leaves real momentum, so the rocket is slow to
    // get going and slow to stop - you have to anticipate and counter-thrust.
    bool thrustR = ActDown(ACT_RIGHT) || (PointerDown() && PointerX() > sys.winWidth * 0.7f);
    bool thrustL = ActDown(ACT_LEFT)  || (PointerDown() && PointerX() < sys.winWidth * 0.3f);
    if (!sys.crashed) {
        if (thrustR) { towerVX += L2_SIDE * dt; sys.score02 -= 2 * dt; }
        if (thrustL) { towerVX -= L2_SIDE * dt; sys.score02 -= 2 * dt; }
        sys.score02 -= 1 * dt;
    }
    towerVX -= towerVX * L2_DAMP * dt;
    towerX += towerVX * dt;

    // Edge push-back (ported edge()).
    if (towerX < sys.winWidth * 0.1f) {
        towerVX += 400 * dt; sys.BGcolorG = sys.BGcolorB = 0.25f;
        if (towerX < sys.winWidth * 0.05f) { towerVX += 4000 * dt; sys.BGcolorG = sys.BGcolorB = 0.01f; }
    } else if (towerX > sys.winWidth * 0.9f) {
        towerVX -= 400 * dt; sys.BGcolorG = sys.BGcolorB = 0.25f;
        if (towerX > sys.winWidth * 0.95f) { towerVX -= 4000 * dt; sys.BGcolorG = sys.BGcolorB = 0.01f; }
    } else {
        sys.BGcolorG = sys.BGcolorB = 1.0f;
    }

    // Meteoroid motion, collisions, cleanup.
    for (int i = 0; i < MAX_METEOROIDS; i++) {
        Meteoroid *m = &junk[i];
        if (!m->active) continue;
        m->wx += m->vx * dt; m->wy += m->vy * dt; m->angle += m->angVel * dt;
        float sy = m->wy + scroll;
        float r = 16.0f * m->scale;

        float gap = TowerGap(m->wx, sy, r, towerX, towerScreenY);
        if (gap < m->gap) m->gap = gap;   // remember the closest approach
        Color tint = (Color){ (unsigned char)(fmaxf(m->red,0)*255),
                              (unsigned char)(m->grn*255), (unsigned char)(m->blu*255), 255 };
        bool hit = gap < 0;
        if (hit && !m->touching && !sys.crashed) {
            PlaySound(assets.itemBreak);
            sys.itemsDestroyed++;
            m->red -= 0.5f;
            sys.score02 -= 50 * m->scale;
            FxBurst((Vector2){ m->wx, sy }, 12, tint, 50, 230, 0.3f, 0.6f, 4, 80);
            FxShake(0.14f);
            combo = 0;                  // a hit breaks the graze chain
        }
        m->touching = hit;

        // Near miss: once it has dropped past the tower untouched but close,
        // reward the precision dodge with a little fuel + a flash.
        if (!sys.crashed && !m->grazeCounted && m->red >= 1.0f &&
            sy > towerScreenY + TOWER_H) {
            m->grazeCounted = true;
            if (m->gap < L2_GRAZE) {
                if (combo < MAX_COMBO) combo++;
                int pts = GRAZE_BONUS * combo;        // chained grazes pay more
                sys.score02 += pts;
                grazeFlash = 0.6f;
                grazeCount++;
                FxBurst((Vector2){ m->wx, sy }, 8 + combo * 2, (Color){ 90, 230, 200, 255 },
                        30, 130 + combo * 30, 0.2f, 0.45f, 3, 0);
                char pbuf[20];
                snprintf(pbuf, sizeof(pbuf), "+%d", pts);
                FxPopup((Vector2){ m->wx, sy - 18 }, pbuf, (Color){ 120, 240, 210, 255 },
                        26.0f + combo * 3.0f);
                if (combo > 1) FxShake(0.05f + combo * 0.02f);
            }
        }

        if (m->red <= 0) {           // destroyed
            PlaySound(assets.itemBreak);
            int loss = (int)(500 * m->scale);
            sys.score02 -= loss;
            sys.itemsDestroyed++;
            FxBurst((Vector2){ m->wx, sy }, 22, tint, 60, 300, 0.4f, 0.9f, 5, 100);
            FxShake(0.2f);
            char pbuf[20];
            snprintf(pbuf, sizeof(pbuf), "-%d", loss);
            FxPopup((Vector2){ m->wx, sy }, pbuf, (Color){ 255, 80, 60, 255 }, 28.0f);
            m->active = false;
            continue;
        }
        if (sy > sys.winHeight * 1.2f) m->active = false; // scrolled off bottom
    }
    if (grazeFlash > 0) grazeFlash -= dt;

    sys.BGScale += 0.005f * dt;
    scroll += L2_SCROLL * dt;
    if (!sys.crashed) scrollTower += (L2_SCROLL / 4.0f) * dt;

    // Level complete when scrolled far enough (tower worldY < -3.4*winH).
    float towerWorldY = sys.winHeight * 0.95f - scroll - scrollTower;
    if (towerWorldY < -sys.winHeight * 3.4f && !sys.crashed) {
        StopSound(assets.fire);
        ChangeScreen(SCREEN_GOAL2);
        return;
    }

    if (sys.score02 <= 0 && !sys.crashed) {
        sys.score02 = 0;
        PlaySound(assets.problem);
        sys.crashed = true;
        StopSound(assets.fire);
        FxExplosion((Vector2){ towerX + TOWER_W * 0.5f, towerScreenY + TOWER_H * 0.5f }, 2.0f);
    }

    if (sys.crashed && (ActPressed(ACT_CONFIRM) || PointerPressed()))
        NextLifeOrGameOver(SCREEN_LEVEL2);
}

static void l2_draw(void) {
    // Scaling background (motion cue).
    Color bg = (Color){ (unsigned char)(sys.BGcolorR*255), (unsigned char)(sys.BGcolorG*255),
                        (unsigned char)(sys.BGcolorB*255), 255 };
    DrawTexturePro(assets.bgSpace,
        (Rectangle){ 0, 0, (float)assets.bgSpace.width, (float)assets.bgSpace.height },
        (Rectangle){ 0, 0, sys.winWidth * sys.BGScale, sys.winHeight * sys.BGScale },
        (Vector2){ 0, 0 }, 0.0f, bg);

    // Parallax starfield for depth (closer stars drift faster).
    for (int i = 0; i < NSTARS; i++) {
        float sy = fmodf(stars[i].y + scroll * stars[i].par, sys.winHeight);
        if (sy < 0) sy += sys.winHeight;
        unsigned char b = (unsigned char)(stars[i].bright * 255);
        DrawRectangle((int)stars[i].x, (int)sy, (int)stars[i].size, (int)stars[i].size,
                      (Color){ 255, 255, 255, b });
    }

    float towerScreenY = sys.winHeight * 0.95f - scrollTower;

    // Tower (top-left anchored, scale 1, like the original).
    DrawTextureEx(tower.image, (Vector2){ towerX, towerScreenY }, 0.0f, 1.0f, WHITE);

    // Flame centered under the tower (origin at the sprite's horizontal middle).
    if (!sys.crashed) {
        DrawTexturePro(assets.fireball.tex, AnimFrame(&assets.fireball),
            (Rectangle){ towerX + TOWER_W * 0.5f, towerScreenY + TOWER_H * 0.86f, 107, 147 },
            (Vector2){ 107 * 0.5f, 0 }, 0.0f, Fade(WHITE, 0.7f));
    }

    // Meteoroids.
    for (int i = 0; i < MAX_METEOROIDS; i++) {
        Meteoroid *m = &junk[i];
        if (!m->active) continue;
        float sy = m->wy + scroll;
        Color tint = (Color){ (unsigned char)(fmaxf(m->red,0)*255),
                              (unsigned char)(m->grn*255), (unsigned char)(m->blu*255), 255 };
        float w = (float)assets.meteoroid.width, h = (float)assets.meteoroid.height;
        DrawTexturePro(assets.meteoroid,
            (Rectangle){ 0, 0, w, h },
            (Rectangle){ m->wx, sy, w * m->scale, h * m->scale },
            (Vector2){ w * m->scale / 2.0f, h * m->scale / 2.0f },
            m->angle * RAD2DEG, tint);
    }

    // "GRAZE!" popup above the tower on a recent near miss.
    if (grazeFlash > 0 && !sys.crashed) {
        unsigned char a = (unsigned char)(255.0f * (grazeFlash / 0.6f));
        DrawTextEx(assets.screenFont, "GRAZE!",
                   (Vector2){ towerX - 20.0f, towerScreenY - 44.0f },
                   (float)assets.screenFont.baseSize, 1, (Color){ 90, 230, 200, a });
    }

    // Score.
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", (int)sys.score02);
    DrawTextEx(assets.scoreFont, buf, (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.1f },
               (float)assets.scoreFont.baseSize, 2, (Color){ 255, 0, 0, 255 });

    if (grazeCount > 0) {
        snprintf(buf, sizeof(buf), "GRAZE x%d", grazeCount);
        DrawTextEx(assets.screenFont, buf,
                   (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.1f + 96.0f },
                   30.0f, 1, (Color){ 90, 230, 200, 255 });
    }
    if (combo > 1) {
        snprintf(buf, sizeof(buf), "COMBO x%d", combo);
        DrawTextEx(assets.screenFont, buf,
                   (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.1f + 134.0f },
                   38.0f, 1, (Color){ 255, 190, 40, 255 });
    }

    if (sys.crashed)
        DrawWrappedText("Out of Fuel!\n\nPress to play again.", assets.screenFont,
                        sys.winWidth * 0.1f, sys.winHeight * 0.3f, sys.winWidth * 0.8f,
                        ALIGN_LEFT, (Color){ 255, 0, 0, 255 });

    DrawLives();
}

static void l2_leave(void) {
    StopSound(assets.fire);
}

const Screen *Level2Screen(void) {
    static const Screen s = { l2_enter, l2_update, l2_draw, l2_leave };
    return &s;
}
