// level1.c - Launch: fuel-fill rhythm (alternate left/right) + countdown, then
// a physics-driven blast-off. Ported from gameLevel01.lua.
#include "game.h"
#include "assets.h"
#include "physics.h"
#include "input.h"
#include "fx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// --- tuning -----------------------------------------------------------------
// L1_GRAVITY holds the tower on the pad (physac) during the countdown; the
// launch itself is now hand-integrated for a controlled, *visible* climb whose
// fuel can run out (px / px-per-second units, tuned by feel).
static float L1_GRAVITY    = 9.81f;
static float L1_TOWER_MASS = 50.0f;
static float L1_CLIMB = 340.0f;   // climb speed while thrusting (px/s) -> ~2.4s ascent
static float L1_BURN  = 300.0f;   // fuel burned per second while thrusting
static float L1_FALL  = 700.0f;   // downward accel (px/s^2) once the tank runs dry
static float L1_CD    = 10.0f;    // fuel-fill countdown (s); shorter = less fuel banked
#define TOWER_SCALE   3.0f
#define ORBIT_Y      (-375.0f)    // clear this height = reached orbit (win)

static float EnvF(const char *k, float def) {
    const char *v = getenv(k);
    return v ? (float)atof(v) : def;
}

static bool  launch;
static float timer, bgAlpha, countDown, flow;
static float vy, launchFuel;      // launch vertical velocity (px/s) + fuel at lift-off
static float vent, fillPulse, startY;
static PhysicsBody groundBody;

#define NL1STARS 70
static Vector2 l1star[NL1STARS];
static float   l1starB[NL1STARS];

// Base of the rocket in screen space (where exhaust / steam / sparks emit).
static Vector2 RocketBase(PhysicsBody tb) {
    return (Vector2){ tb->position.x, tb->position.y + tower.height * TOWER_SCALE * 0.45f };
}

// Tactile feedback for a good rhythm press: sparks, a steam puff, a bar pulse.
static void PressFeedback(PhysicsBody tb) {
    Vector2 b = RocketBase(tb);
    FxBurst(b, 7, (Color){ 255, 205, 90, 255 }, 60, 180, 0.18f, 0.4f, 4, 140);
    FxSmoke(b, (Vector2){ 0, -25 }, 2, (Color){ 210, 210, 216, 90 }, 12, 0.5f);
    FxShake(0.04f);
    fillPulse = 1.0f;
}

static void l1_enter(void) {
    L1_GRAVITY      = EnvF("L1_G", 9.81f);
    L1_TOWER_MASS   = EnvF("L1_M", 50.0f);
    L1_CLIMB        = EnvF("L1_CLIMB", 340.0f);
    L1_BURN         = EnvF("L1_BURN", 300.0f);
    L1_FALL         = EnvF("L1_FALL", 700.0f);
    L1_CD           = EnvF("L1_CD", 10.0f);
    launch = false;
    timer = 1.0f;
    bgAlpha = 0.0f;
    countDown = L1_CD;
    flow = 0.0f;
    vy = 0.0f;
    launchFuel = 0.0f;
    vent = 0.0f;
    fillPulse = 0.0f;
    startY = sys.winHeight * 0.6f;
    sys.crashed = false;
    if (getenv("L1_FUEL")) sys.score01 = (float)atof(getenv("L1_FUEL")); // debug: preset tank

    for (int i = 0; i < NL1STARS; i++) {
        l1star[i] = (Vector2){ (float)GetRandomValue(0, (int)sys.winWidth),
                               (float)GetRandomValue(0, (int)(sys.winHeight * 0.7f)) };
        l1starB[i] = (float)GetRandomValue(40, 100) / 100.0f;
    }

    PhysicsReset();
    PhysicsSetGravity(0, L1_GRAVITY);

    // Static ground (disabled body = immovable wall in physac).
    groundBody = CreatePhysicsBodyRectangle(
        (Vector2){ sys.winWidth / 2.0f, sys.winHeight * 0.95f },
        sys.winWidth, sys.winHeight * 0.25f, 1.0f);
    groundBody->enabled = false;

    // Dynamic tower.
    PhysicsBody tb = CreatePhysicsBodyRectangle(
        (Vector2){ sys.winWidth * 0.681f, sys.winHeight * 0.6f },
        tower.width * TOWER_SCALE, tower.height * TOWER_SCALE, 1.0f);
    tb->mass = L1_TOWER_MASS;
    tb->inverseMass = 1.0f / L1_TOWER_MASS;
    tb->freezeOrient = true;   // keep it upright on the pad
    tower.body = tb;
}

static void l1_update(float dt) {
    PhysicsStep();
    PhysicsBody tb = (PhysicsBody)tower.body;
    timer += dt;
    if (bgAlpha < 1.0f && timer > 1.0f) bgAlpha = FadeIn(dt, bgAlpha, 0.9f);

    if (tb->position.y < ORBIT_Y) { ChangeScreen(SCREEN_GOAL1); return; }

    // Launch: hand-integrated so the climb is visible and the tank can run dry.
    // While fuel lasts the tower climbs steadily; when it's gone, gravity wins
    // and it falls back -> if it sinks back to the pad it has stalled (crash).
    if (launch && !sys.crashed) {
        if (!IsSoundPlaying(assets.launch)) PlaySound(assets.launch);
        if (sys.score01 > 0) {
            vy = -L1_CLIMB;                 // steady, watchable ascent
            sys.score01 -= L1_BURN * dt;    // burning fuel to climb
            if (sys.score01 < 0) sys.score01 = 0;
            AnimAdvance(&assets.fireball, 10.0f * dt);
            // Thick exhaust plume + a lingering smoke trail.
            Vector2 ex = { tb->position.x, tb->position.y + tower.height * TOWER_SCALE * 0.5f };
            FxExhaust(ex, (Vector2){ 0, 440 }, (Color){ 255, 185, 55, 255 }, 36, 5);
            FxExhaust(ex, (Vector2){ 0, 300 }, (Color){ 255, 95, 30, 255 }, 24, 2);
            FxSmoke(ex, (Vector2){ 0, 120 }, 2, (Color){ 120, 110, 110, 95 }, 18, 1.0f);
            FxShake(0.06f);                 // continuous launch rumble
        } else {
            vy += L1_FALL * dt;             // out of fuel: stall and drop
        }
        tb->position.y += vy * dt;

        if (sys.score01 <= 0 && tb->position.y > sys.winHeight * 0.6f) {
            sys.score01 = 0;
            sys.crashed = true;             // fell back to the pad without reaching orbit
            FxExplosion((Vector2){ tb->position.x, tb->position.y }, 2.4f);
        }
    }

    if (fillPulse > 0) fillPulse -= dt * 3.0f;

    // Countdown drives fuel accumulation from the rhythm "flow".
    if (countDown > 0 && !sys.crashed) {
        countDown -= dt;
        if (countDown < 0) countDown = 0;
        if ((int)floorf(countDown) == 9 && !IsSoundPlaying(assets.countdown))
            PlaySound(assets.countdown);
        flow -= (flow * 3.2f) * dt * (sys.level * 0.8f);
        sys.score01 += flow;

        // Steam venting from the engines, and a build-up shudder as T-0 nears.
        vent -= dt;
        if (vent <= 0) {
            Vector2 b = RocketBase(tb);
            b.x += (float)GetRandomValue(-1, 1) * tower.width * TOWER_SCALE * 0.3f;
            FxSmoke(b, (Vector2){ (float)GetRandomValue(-90, 90), -20 }, 3,
                    (Color){ 210, 210, 216, 95 }, 16, 0.9f);
            vent = 0.045f;
        }
        if (countDown < 3.0f) FxShake((3.0f - countDown) / 3.0f * 0.05f);
    } else if (!sys.crashed && !launch) {
        launch = true;
        flow = 10.0f;
        countDown = 0;
        launchFuel = sys.score01;           // remember the tank size for the gauge
        if (launchFuel < 1.0f) launchFuel = 1.0f;
        tb->enabled = false;                // hand off motion to the manual climb
        vy = 0.0f;
        // Ignition: a billowing ground cloud + fireball + a hard kick.
        Vector2 b = RocketBase(tb);
        FxBurst(b, 64, (Color){ 255, 205, 70, 255 }, 80, 440, 0.4f, 0.95f, 7, 280);
        FxSmoke(b, (Vector2){ 0, 50 }, 26, (Color){ 195, 195, 200, 150 }, 24, 1.4f);
        FxSmoke(b, (Vector2){ 0, 30 }, 16, (Color){ 110, 105, 105, 130 }, 26, 1.3f);
        FxShake(0.7f);
        PlaySound(assets.launch);
    }

    // Rhythm input: alternate left/right to add flow.
    if (!sys.crashed && countDown > 0) {
        if (ActPressed(ACT_RIGHT) && sys.launch == LAUNCH_LEFT) {
            flow += GetRandomValue(8, 16) / 10.0f; sys.launch = LAUNCH_RIGHT; PressFeedback(tb);
        } else if (ActPressed(ACT_LEFT) && sys.launch == LAUNCH_RIGHT) {
            flow += GetRandomValue(8, 16) / 10.0f; sys.launch = LAUNCH_LEFT; PressFeedback(tb);
        }
        // Pointer zones (mouse/touch) mirror the original.
        if (PointerPressed()) {
            float x = PointerX();
            if (x < sys.winWidth * 0.3f && sys.launch == LAUNCH_LEFT) {
                flow += GetRandomValue(8, 16) / 10.0f; sys.launch = LAUNCH_RIGHT; PressFeedback(tb);
            } else if (x > sys.winWidth * 0.7f && sys.launch == LAUNCH_RIGHT) {
                flow += GetRandomValue(8, 16) / 10.0f; sys.launch = LAUNCH_LEFT; PressFeedback(tb);
            }
        }
    }

    if (sys.crashed && (ActPressed(ACT_CONFIRM) || PointerPressed()))
        NextLifeOrGameOver(SCREEN_LEVEL1);
}

static void l1_draw(void) {
    PhysicsBody tb = (PhysicsBody)tower.body;

    // Climb progress (0 on the pad -> 1 at orbit) drives the ascent visuals.
    float climb = (startY - tb->position.y) / (startY - ORBIT_Y);
    if (climb < 0) climb = 0;
    if (climb > 1) climb = 1;

    // Background city, darkening toward space as we rise.
    DrawTexturePro(assets.bgCity,
        (Rectangle){ 0, 0, (float)assets.bgCity.width, (float)assets.bgCity.height },
        (Rectangle){ 0, 0, sys.winWidth, sys.winHeight },
        (Vector2){ 0, 0 }, 0.0f, Fade(WHITE, bgAlpha));
    if (climb > 0)
        DrawRectangle(0, 0, (int)sys.winWidth, (int)sys.winHeight,
                      Fade((Color){ 4, 6, 22, 255 }, climb * 0.85f * bgAlpha));
    // Stars fade in once we're high enough.
    float starA = (climb - 0.25f) / 0.5f;
    if (starA > 0) {
        if (starA > 1) starA = 1;
        for (int i = 0; i < NL1STARS; i++)
            DrawRectangle((int)l1star[i].x, (int)l1star[i].y, 2, 2,
                          Fade(WHITE, starA * l1starB[i] * bgAlpha));
    }

    // Ground slab.
    DrawRectangle(0, (int)(sys.winHeight * 0.95f - sys.winHeight * 0.125f),
                  (int)sys.winWidth, (int)(sys.winHeight * 0.25f),
                  (Color){ 38, 92, 66, 102 });

    // Tower (centered on body, scaled, rotated).
    DrawTexturePro(tower.image,
        (Rectangle){ 0, 0, tower.width, tower.height },
        (Rectangle){ tb->position.x, tb->position.y,
                     tower.width * TOWER_SCALE, tower.height * TOWER_SCALE },
        (Vector2){ tower.width * TOWER_SCALE / 2.0f, tower.height * TOWER_SCALE / 2.0f },
        tb->orient * RAD2DEG, Fade(WHITE, bgAlpha));

    // Engine "charge" glow on the pad: flares brighter the harder you mash.
    if (!launch && !sys.crashed) {
        Vector2 b = RocketBase(tb);
        float g = flow / 28.0f; if (g > 1) g = 1;
        float rad = 16.0f + 46.0f * g + fillPulse * 22.0f;
        DrawCircleV(b, rad, Fade((Color){ 255, 150, 40, 255 }, 0.45f * g * bgAlpha));
        DrawCircleV(b, rad * 0.5f, Fade((Color){ 255, 235, 160, 255 }, 0.6f * g * bgAlpha));
    }

    // Fireball under the tower while launching.
    if (launch) {
        DrawTexturePro(assets.fireball.tex, AnimFrame(&assets.fireball),
            (Rectangle){ tb->position.x, tb->position.y + tower.height * TOWER_SCALE / 2.0f,
                         107 * TOWER_SCALE, 147 * TOWER_SCALE },
            (Vector2){ 107 * TOWER_SCALE / 2.0f, 0 }, 0.0f, Fade(WHITE, 0.98f));
    }

    // Fuel bar: rhythm "flow" while filling; the draining tank while launching.
    float barX = sys.winWidth * 0.1f, barY = sys.winHeight * 0.84f;
    float barW = sys.winWidth * 0.8f, barH = sys.winHeight * 0.1f;
    DrawRectangleLinesEx((Rectangle){ barX, barY, barW, barH }, 2, Fade((Color){255,230,230,255}, bgAlpha));
    float fillW; Color fillCol;
    if (launch) {
        float frac = launchFuel > 0 ? sys.score01 / launchFuel : 0;
        fillW = frac * barW;
        // Amber with a healthy reserve, red once it's about to stall.
        fillCol = (sys.score01 > L1_BURN * 0.6f) ? (Color){ 255, 170, 40, 255 }
                                                 : (Color){ 255, 40, 40, 255 };
    } else {
        fillW = flow * sys.winWidth * 0.1f;
        fillCol = (Color){ 255, 26, 26, 255 };
    }
    if (fillW > barW) fillW = barW;
    if (fillW < 0) fillW = 0;
    DrawRectangle((int)barX, (int)barY, (int)fillW, (int)barH, Fade(fillCol, bgAlpha));
    // Pulse the bar white on each good press for tactile feedback.
    if (fillPulse > 0)
        DrawRectangle((int)barX, (int)barY, (int)fillW, (int)barH,
                      Fade(WHITE, fillPulse * 0.35f * bgAlpha));

    // Score + countdown.
    char buf[32];
    Color red = Fade((Color){ 255, 0, 0, 255 }, bgAlpha);
    snprintf(buf, sizeof(buf), "%d", (int)sys.score01);
    DrawTextEx(assets.scoreFont, buf, (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.1f },
               (float)assets.scoreFont.baseSize, 2, red);
    snprintf(buf, sizeof(buf), "%d", (int)countDown);
    DrawTextEx(assets.countDownFont, buf, (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.65f },
               (float)assets.countDownFont.baseSize, 2, red);

    if (countDown == 0 && !sys.crashed)
        DrawTextEx(assets.screenFont, "Launch!",
                   (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.3f },
                   (float)assets.screenFont.baseSize, 2, red);
    if (sys.crashed)
        DrawWrappedText("Out of fuel - stalled!\n\nPress to try again.", assets.screenFont,
                        sys.winWidth * 0.1f, sys.winHeight * 0.3f, sys.winWidth * 0.8f, ALIGN_LEFT, red);

    DrawLives();
}

static void l1_leave(void) {
    PhysicsReset();
    tower.body = NULL;
}

const Screen *Level1Screen(void) {
    static const Screen s = { l1_enter, l1_update, l1_draw, l1_leave };
    return &s;
}
