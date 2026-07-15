// level1.c - Launch: a DDR-style "enter the launch codes" minigame. Four-arrow
// notes scroll up their lanes toward a receptor line; hitting the matching arrow
// in time banks fuel into sys.score01. When the whole code is entered, the tower
// ignites and the (unchanged) hand-integrated climb burns that fuel - enough
// reaches orbit (win -> Goal1), too little stalls and crashes (retry).
#include "game.h"
#include "assets.h"
#include "physics.h"
#include "input.h"
#include "fx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// --- tuning -----------------------------------------------------------------
// L1_GRAVITY holds the tower on the pad (physac) during code entry; the launch
// itself is hand-integrated for a controlled, *visible* climb whose fuel can run
// out (px / px-per-second units, tuned by feel).
static float L1_GRAVITY    = 9.81f;
static float L1_TOWER_MASS = 50.0f;
static float L1_CLIMB = 340.0f;   // climb speed while thrusting (px/s) -> ~2.4s ascent
static float L1_BURN  = 300.0f;   // fuel burned per second while thrusting
static float L1_FALL  = 700.0f;   // downward accel (px/s^2) once the tank runs dry

// DDR launch-code chart tuning.
static float L1_BPM          = 132.0f; // fixed tempo of the launch-code chart
static float L1_LEAD         = 1.9f;   // seconds a note takes to scroll to the receptor
static float L1_PERFECT      = 0.06f;  // |timing| for a Perfect hit
static float L1_GOOD         = 0.13f;  // |timing| for a Good hit (also the miss cutoff)
static float L1_FUEL_PERFECT = 52.0f;  // fuel banked for a Perfect (before combo)
static float L1_FUEL_GOOD    = 30.0f;  // fuel banked for a Good (before combo)
static int   L1_NOTES_BASE   = 22;     // notes on round 1 (grows with sys.level)
static float L1_PREROLL      = 2.6f;   // "GET READY" grace period before the first note

#define TOWER_SCALE   3.0f
#define ORBIT_Y      (-375.0f)    // clear this height = reached orbit (win)
#define L1_LANES      4           // 0=LEFT, 1=DOWN, 2=UP, 3=RIGHT (left->right on screen)
#define L1_MAX_NOTES  64

static float EnvF(const char *k, float def) {
    const char *v = getenv(k);
    return v ? (float)atof(v) : def;
}

typedef struct { int lane; float time; bool judged; } Note;

static bool  launch;
static float timer, bgAlpha, flow;   // flow = decaying pad-glow pulse (flares on a hit)
static float vy, launchFuel;         // launch vertical velocity (px/s) + fuel at lift-off
static float vent, fillPulse, startY;
static PhysicsBody groundBody;

static Note  notes[L1_MAX_NOTES];
static int   noteCount, judgedCount, combo, hits;
static float chartTime, fuelTarget, scrollSpeed;
static float laneFlash[L1_LANES];    // per-lane receptor flash (1 on hit, decays)

#define NL1STARS 70
static Vector2 l1star[NL1STARS];
static float   l1starB[NL1STARS];

// Lane arrow colours + the DrawPoly rotation that points a triangle each way.
static const Color LANE_COL[L1_LANES] = {
    { 255,  90, 170, 255 },  // LEFT  - pink
    {  70, 200, 255, 255 },  // DOWN  - cyan
    { 130, 230, 130, 255 },  // UP    - green
    { 255, 175,  70, 255 },  // RIGHT - amber
};
static const float LANE_ROT[L1_LANES] = { 180.0f, 90.0f, -90.0f, 0.0f };
static const Action LANE_ACT[L1_LANES] = { ACT_LEFT, ACT_DOWN, ACT_THRUST, ACT_RIGHT };

static float ReceptorY(void) { return sys.winHeight * 0.20f; }
static float LaneX(int i)    { return sys.winWidth * (0.15f + i * 0.095f); }

// Base of the rocket in screen space (where exhaust / steam / sparks emit).
static Vector2 RocketBase(PhysicsBody tb) {
    return (Vector2){ tb->position.x, tb->position.y + tower.height * TOWER_SCALE * 0.45f };
}

// Register a successful note hit: bank combo-scaled fuel + spark/popup feedback.
static void ChartHit(int i, bool perfect) {
    notes[i].judged = true; judgedCount++; combo++; hits++;
    float mult = 1.0f + (combo < 8 ? combo : 8) * 0.05f;   // combo ramps fuel up to 1.4x
    sys.score01 += (perfect ? L1_FUEL_PERFECT : L1_FUEL_GOOD) * mult;
    fillPulse = 1.0f;
    flow = 12.0f;                                          // flare the pad glow
    laneFlash[notes[i].lane] = 1.0f;                       // light up this lane's receptor
    Vector2 rp = { LaneX(notes[i].lane), ReceptorY() };
    FxBurst(rp, perfect ? 10 : 6,
            perfect ? (Color){ 255, 240, 150, 255 } : (Color){ 180, 220, 255, 255 },
            60, 220, 0.2f, 0.5f, 4, 120);
    FxPopup((Vector2){ rp.x, rp.y - 34 }, perfect ? "PERFECT!" : "GOOD",
            perfect ? (Color){ 255, 230, 120, 255 } : (Color){ 150, 220, 255, 255 },
            perfect ? 34.0f : 28.0f);
    FxShake(perfect ? 0.05f : 0.03f);
}

// Register a missed note: break the combo, no fuel banked.
static void ChartMiss(int i) {
    notes[i].judged = true; judgedCount++; combo = 0;
    Vector2 rp = { LaneX(notes[i].lane), ReceptorY() };
    FxPopup((Vector2){ rp.x, rp.y - 34 }, "MISS", (Color){ 255, 90, 90, 255 }, 30.0f);
    FxShake(0.12f);
}

// A press in `lane`: judge the nearest un-judged note within the Good window.
// Stray presses (no note in range) are ignored - lenient, no penalty.
static void TryHitLane(int lane) {
    int best = -1; float bestDt = L1_GOOD + 1.0f;
    for (int i = 0; i < noteCount; i++) {
        if (notes[i].judged || notes[i].lane != lane) continue;
        float d = fabsf(notes[i].time - chartTime);
        if (d <= L1_GOOD && d < bestDt) { best = i; bestDt = d; }
    }
    if (best >= 0) ChartHit(best, bestDt <= L1_PERFECT);
}

// Ignition: hand off to the manual climb, throwing a ground cloud + fireball.
static void Ignite(PhysicsBody tb) {
    launch = true;
    launchFuel = sys.score01;           // remember the tank size for the gauge
    if (launchFuel < 1.0f) launchFuel = 1.0f;
    tb->enabled = false;                // hand off motion to the manual climb
    vy = 0.0f;
    Vector2 b = RocketBase(tb);
    FxBurst(b, 64, (Color){ 255, 205, 70, 255 }, 80, 440, 0.4f, 0.95f, 7, 280);
    FxSmoke(b, (Vector2){ 0, 50 }, 26, (Color){ 195, 195, 200, 150 }, 24, 1.4f);
    FxSmoke(b, (Vector2){ 0, 30 }, 16, (Color){ 110, 105, 105, 130 }, 26, 1.3f);
    FxShake(0.7f);
    PlaySound(assets.launch);
}

static void l1_enter(void) {
    L1_GRAVITY      = EnvF("L1_G", 9.81f);
    L1_TOWER_MASS   = EnvF("L1_M", 50.0f);
    L1_CLIMB        = EnvF("L1_CLIMB", 340.0f);
    L1_BURN         = EnvF("L1_BURN", 300.0f);
    L1_FALL         = EnvF("L1_FALL", 700.0f);
    L1_BPM          = EnvF("L1_BPM", 132.0f);
    L1_LEAD         = EnvF("L1_LEAD", 1.9f);
    L1_PERFECT      = EnvF("L1_PERFECT", 0.06f);
    L1_GOOD         = EnvF("L1_GOOD", 0.13f);
    L1_FUEL_PERFECT = EnvF("L1_FUEL_PERFECT", 52.0f);
    L1_FUEL_GOOD    = EnvF("L1_FUEL_GOOD", 30.0f);
    L1_NOTES_BASE   = (int)EnvF("L1_NOTES", 22.0f);
    L1_PREROLL      = EnvF("L1_PREROLL", 2.6f);

    launch = false;
    timer = 1.0f;
    bgAlpha = 0.0f;
    flow = 0.0f;
    vy = 0.0f;
    launchFuel = 0.0f;
    vent = 0.0f;
    fillPulse = 0.0f;
    startY = sys.winHeight * 0.6f;
    sys.crashed = false;
    if (getenv("L1_FUEL")) sys.score01 = (float)atof(getenv("L1_FUEL")); // debug: preset tank

    // Build the fixed-tempo launch-code chart. Higher rounds pack notes tighter
    // and add a few more, so later loops demand faster, cleaner code entry.
    float beat = (60.0f / L1_BPM) / (1.0f + (sys.level - 1) * 0.12f);
    noteCount = L1_NOTES_BASE + (sys.level - 1) * 4;
    if (noteCount > L1_MAX_NOTES) noteCount = L1_MAX_NOTES;
    if (noteCount < 1) noteCount = 1;
    for (int i = 0; i < noteCount; i++) {
        notes[i].lane = GetRandomValue(0, L1_LANES - 1);
        notes[i].time = L1_LEAD + i * beat;   // first note scrolls in over L1_LEAD s
        notes[i].judged = false;
    }
    judgedCount = 0; combo = 0; hits = 0;
    chartTime = -L1_PREROLL;   // "GET READY" lead-in: notes hold off-screen, then approach
    for (int i = 0; i < L1_LANES; i++) laneFlash[i] = 0.0f;
    scrollSpeed = (sys.winHeight - ReceptorY()) / L1_LEAD;
    fuelTarget = noteCount * L1_FUEL_PERFECT;
    if (fuelTarget < 1.0f) fuelTarget = 1.0f;
    PlaySound(assets.countdown);   // Apollo "T-minus" cue as the code entry opens

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
    if (flow > 0) { flow -= flow * 4.0f * dt; if (flow < 0.01f) flow = 0; }
    for (int L = 0; L < L1_LANES; L++)
        if (laneFlash[L] > 0) laneFlash[L] -= dt * 3.0f;

    // Launch-code entry: scroll the chart, judge presses, auto-miss stragglers,
    // then ignite once every note has been resolved.
    if (!launch && !sys.crashed) {
        chartTime += dt;

        // Idle engine steam while the crew enters the code.
        vent -= dt;
        if (vent <= 0) {
            Vector2 b = RocketBase(tb);
            b.x += (float)GetRandomValue(-1, 1) * tower.width * TOWER_SCALE * 0.3f;
            FxSmoke(b, (Vector2){ (float)GetRandomValue(-90, 90), -20 }, 3,
                    (Color){ 210, 210, 216, 95 }, 16, 0.9f);
            vent = 0.06f;
        }

        for (int L = 0; L < L1_LANES; L++)
            if (ActPressed(LANE_ACT[L])) TryHitLane(L);
        // Pointer/touch: hit the lane nearest the tap.
        if (PointerPressed()) {
            float px = PointerX(); int best = 0; float bd = 1e9f;
            for (int L = 0; L < L1_LANES; L++) {
                float d = fabsf(px - LaneX(L));
                if (d < bd) { bd = d; best = L; }
            }
            if (bd < sys.winWidth * 0.06f) TryHitLane(best);
        }

        for (int i = 0; i < noteCount; i++)
            if (!notes[i].judged && (chartTime - notes[i].time) > L1_GOOD) ChartMiss(i);

        if (judgedCount >= noteCount) Ignite(tb);
    }

    if (sys.crashed && (ActPressed(ACT_CONFIRM) || PointerPressed()))
        NextLifeOrGameOver(SCREEN_LEVEL1);
}

// Draw a filled arrow (note) or outline (receptor) for `lane` at center `c`.
static void DrawArrow(Vector2 c, int lane, float r, bool filled, Color col) {
    if (filled) {
        DrawPoly(c, 3, r, LANE_ROT[lane], col);
        DrawPolyLinesEx(c, 3, r, LANE_ROT[lane], 2.0f, Fade(WHITE, 0.5f * ((float)col.a / 255.0f)));
    } else {
        DrawPolyLinesEx(c, 3, r, LANE_ROT[lane], 3.0f, col);
    }
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

    // Engine "charge" glow on the pad: flares on each launch-code hit.
    if (!launch && !sys.crashed) {
        Vector2 b = RocketBase(tb);
        float g = flow / 14.0f; if (g > 1) g = 1;
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

    // Launch-code lanes: target zone + receptors + scrolling notes + readout.
    if (!launch && !sys.crashed) {
        float ry = ReceptorY();
        float ar = sys.winHeight * 0.038f;
        float bandL = LaneX(0) - ar * 1.7f;
        float bandR = LaneX(L1_LANES - 1) + ar * 1.7f;
        float bandW = bandR - bandL;

        DrawWrappedText("ENTER LAUNCH CODE", assets.messageFont,
                        0, sys.winHeight * 0.03f, sys.winWidth, ALIGN_CENTER, Fade(WHITE, bgAlpha));

        // Target zone: a faint band across the lanes framed by edge lines, with a
        // bright hit-line through the middle so it's obvious WHERE to strike.
        DrawRectangle((int)bandL, (int)(ry - ar * 1.35f), (int)bandW, (int)(ar * 2.7f),
                      Fade((Color){ 255, 255, 255, 255 }, 0.08f * bgAlpha));
        DrawRectangle((int)bandL, (int)(ry - ar * 1.35f), (int)bandW, 2,
                      Fade((Color){ 255, 255, 255, 255 }, 0.35f * bgAlpha));
        DrawRectangle((int)bandL, (int)(ry + ar * 1.35f - 2), (int)bandW, 2,
                      Fade((Color){ 255, 255, 255, 255 }, 0.35f * bgAlpha));
        DrawRectangle((int)bandL, (int)ry - 2, (int)bandW, 4,
                      Fade((Color){ 255, 255, 255, 255 }, 0.85f * bgAlpha));   // the hit line

        // Receptors: a dim filled arrow + a bright outline, flaring on a fresh hit.
        for (int L = 0; L < L1_LANES; L++) {
            Vector2 c = { LaneX(L), ry };
            float f = laneFlash[L]; if (f < 0) f = 0;
            DrawArrow(c, L, ar * 0.92f, true,  Fade(LANE_COL[L], (0.16f + 0.5f * f) * bgAlpha));
            DrawArrow(c, L, ar * 1.15f, false, Fade(LANE_COL[L], (0.6f + 0.4f * f) * bgAlpha));
        }

        // Scrolling notes.
        for (int i = 0; i < noteCount; i++) {
            if (notes[i].judged) continue;
            float ny = ry + (notes[i].time - chartTime) * scrollSpeed;
            if (ny < -ar || ny > sys.winHeight + ar) continue;
            DrawArrow((Vector2){ LaneX(notes[i].lane), ny }, notes[i].lane, ar, true,
                      Fade(LANE_COL[notes[i].lane], bgAlpha));
        }

        // Lead-in prompt during the pre-roll, then the combo readout once running.
        if (chartTime < 0.0f) {
            DrawWrappedText("GET READY", assets.messageFont, 0, ry + sys.winHeight * 0.16f,
                            sys.winWidth, ALIGN_CENTER, Fade((Color){ 255, 220, 120, 255 }, bgAlpha));
        } else if (combo > 1) {
            char cb[32]; snprintf(cb, sizeof(cb), "COMBO x%d", combo);
            DrawWrappedText(cb, assets.screenFont, 0, ry + sys.winHeight * 0.16f,
                            sys.winWidth, ALIGN_CENTER, Fade((Color){ 255, 220, 120, 255 }, bgAlpha));
        }
    }

    // Fuel bar: banked fuel while entering the code; the draining tank while launching.
    float barX = sys.winWidth * 0.1f, barY = sys.winHeight * 0.84f;
    float barW = sys.winWidth * 0.8f, barH = sys.winHeight * 0.1f;
    DrawRectangleLinesEx((Rectangle){ barX, barY, barW, barH }, 2, Fade((Color){255,230,230,255}, bgAlpha));
    float frac, fillW; Color fillCol;
    if (launch) {
        frac = launchFuel > 0 ? sys.score01 / launchFuel : 0;
        fillCol = (sys.score01 > L1_BURN * 0.6f) ? (Color){ 255, 170, 40, 255 }
                                                 : (Color){ 255, 40, 40, 255 };
    } else {
        frac = sys.score01 / fuelTarget;
        fillCol = (Color){ 100, 220, 120, 255 };   // filling green as the code banks fuel
    }
    fillW = frac * barW;
    if (fillW > barW) fillW = barW;
    if (fillW < 0) fillW = 0;
    DrawRectangle((int)barX, (int)barY, (int)fillW, (int)barH, Fade(fillCol, bgAlpha));
    if (fillPulse > 0)
        DrawRectangle((int)barX, (int)barY, (int)fillW, (int)barH,
                      Fade(WHITE, fillPulse * 0.35f * bgAlpha));

    // Fuel readout: a big remaining-fuel number during launch, a compact label
    // during code entry (so it doesn't sit on top of the note lanes).
    char buf[48];
    Color red = Fade((Color){ 255, 0, 0, 255 }, bgAlpha);
    if (launch) {
        snprintf(buf, sizeof(buf), "%d", (int)sys.score01);
        DrawTextEx(assets.scoreFont, buf, (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.1f },
                   (float)assets.scoreFont.baseSize, 2, red);
    } else {
        snprintf(buf, sizeof(buf), "FUEL: %d", (int)sys.score01);
        DrawTextEx(assets.screenFont, buf, (Vector2){ barX, barY - sys.winHeight * 0.06f },
                   (float)assets.screenFont.baseSize, 2, Fade((Color){ 200, 255, 210, 255 }, bgAlpha));
    }

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
