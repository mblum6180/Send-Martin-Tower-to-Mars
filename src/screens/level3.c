// level3.c - Mars: Lunar-Lander on randomly generated terrain. Land slow and
// roughly upright for a bonus, else crash. Ported from gameLevel03.lua.
//
// physac drives the lander (gravity + thrust + torque). Box2D's chain-shape
// terrain has no physac equivalent, so the terrain is our own polyline and
// contact is detected by testing the lander's lower corners against it
// (replacing beginContact/endContact).
#include "game.h"
#include "assets.h"
#include "physics.h"
#include "input.h"
#include "fx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXPTS 96
static Vector2 surf[MAXPTS];  // terrain surface, screen coords, x ascending
static int     surfN;
static bool    segPad[MAXPTS];// segment i (surf[i]..surf[i+1]) is a flat landing pad
static float   landingSpeed;
static bool    landingSpeedSet;
static bool    contact;       // lander currently touching terrain (edge-detect)
static float   padMult;       // touchdown bonus multiplier (1 = off-pad)
static float   startFade;      // level-start fade-from-black (1 -> 0)
static bool    onPad;         // landed on a marked pad (for the result text)

// --- tuning (physac units; tuned by feel) -----------------------------------
static float L3_G      = 3.72f;
static float L3_MAIN   = 2.0f;
static float L3_SIDE   = 0.5f;
static float L3_TORQUE = 0.4f;
static float L3_LANDV  = 1.0f;   // max downward velocity (physac units) for a safe landing
static float EnvF(const char *k, float def){ const char*v=getenv(k); return v?(float)atof(v):def; }

static int Rndi(int a, int b) { if (a > b) { int t=a; a=b; b=t; } return GetRandomValue(a, b); }

static void GenTerrain(void) {
    float W = sys.winWidth, H = sys.winHeight, OFF = H * 0.5f;
    surfN = 0;
    for (int i = 0; i < MAXPTS; i++) segPad[i] = false;
    surf[surfN++] = (Vector2){ 0, OFF };
    float prevY = (float)Rndi((int)(H*0.1f), (int)(H*0.3f));
    surf[surfN++] = (Vector2){ 1, OFF + prevY };

    int slices = Rndi(18, 30);
    int padCount = 0;
    for (int i = 1; i <= slices && surfN < MAXPTS - 2; i++) {
        float x = i * (float)Rndi((int)(W/slices) - 1, (int)(W/slices));
        bool  flat = (Rndi(1, 100) < 14);                       // chance of a flat landing pad
        float y;
        if (flat) y = prevY;                                    // horizontal segment = pad
        else y = (float)Rndi((int)(prevY - Rndi(0, (int)(H*0.1f*sys.level))), (int)(H*0.32f));
        if (y < 0) y = 0;
        surf[surfN++] = (Vector2){ x, OFF + y };
        if (flat) { segPad[surfN - 2] = true; padCount++; }     // segment prev->new is flat
        prevY = y;
    }
    surf[surfN++] = (Vector2){ W, OFF + (float)Rndi(0, (int)(H*0.5f)) };

    // Guarantee at least one pad so the terrain always has a target.
    if (padCount == 0 && surfN > 4) {
        int i = Rndi(2, surfN - 3);
        surf[i + 1].y = surf[i].y;
        segPad[i] = true;
    }
}

// Bonus multiplier for landing on segment `seg` (1 = not a pad). Narrower pads
// are harder, so they pay more.
static float PadMultiplier(int seg) {
    if (seg < 0 || seg >= surfN - 1 || !segPad[seg]) return 1.0f;
    float w = surf[seg + 1].x - surf[seg].x;
    float m = 2.0f + (60.0f - w) / 30.0f;   // ~2x wide, up to ~3x narrow
    if (m < 2.0f) m = 2.0f;
    if (m > 3.0f) m = 3.0f;
    return m;
}

// Index of the terrain segment under world x (-1 if off the ends).
static int SegmentAt(float x) {
    for (int i = 0; i < surfN - 1; i++)
        if (x >= surf[i].x && x <= surf[i + 1].x) return i;
    return -1;
}

// Terrain surface Y at world x (linear interpolation between points).
static float TerrainY(float x) {
    if (x <= surf[0].x) return surf[0].y;
    if (x >= surf[surfN-1].x) return surf[surfN-1].y;
    for (int i = 0; i < surfN - 1; i++) {
        if (x >= surf[i].x && x <= surf[i+1].x) {
            float t = (x - surf[i].x) / (surf[i+1].x - surf[i].x + 0.0001f);
            return surf[i].y + t * (surf[i+1].y - surf[i].y);
        }
    }
    return surf[surfN-1].y;
}

static void l3_enter(void) {
    L3_G      = EnvF("L3_G", 3.72f);
    L3_MAIN   = EnvF("L3_MAIN", 6.0f);
    L3_SIDE   = EnvF("L3_SIDE", 1.2f);
    L3_TORQUE = EnvF("L3_TORQUE", 0.6f);

    sys.score03 = sys.score02;
    landingSpeed = 0; landingSpeedSet = false; contact = false;
    padMult = 1.0f; onPad = false;
    startFade = 1.0f;
    SetSoundVolume(assets.fire, 1.0f);
    GenTerrain();

    PhysicsReset();
    PhysicsSetGravity(0, L3_G);

    PhysicsBody tb = CreatePhysicsBodyRectangle(
        (Vector2){ (float)Rndi(150, (int)sys.winWidth - 150), 0 },
        tower.width, tower.height, 1.0f);
    tb->mass = 50.0f; tb->inverseMass = 1.0f / 50.0f;
    tb->inertia = 500.0f; tb->inverseInertia = 1.0f / 500.0f;
    tb->restitution = 0.3f;
    tower.body = tb;
}

// Lander control (ported input()).
static void l3_input(void) {
    PhysicsBody tb = (PhysicsBody)tower.body;
    if (ActDown(ACT_ROTL))      PhysicsAddTorque(tb,  L3_TORQUE);
    else if (ActDown(ACT_ROTR)) PhysicsAddTorque(tb, -L3_TORQUE);

    if (sys.moveGas > 0) {
        float a = tb->orient - 1.57f;
        PhysicsAddForce(tb, (Vector2){ L3_MAIN * cosf(a), L3_MAIN * sinf(a) });
        tower.fire = true;
    } else {
        tower.fire = false;
    }
    if (sys.moveLeft > 0) {
        float a = tb->orient + 3.14f;
        PhysicsAddForce(tb, (Vector2){ L3_SIDE * cosf(a), L3_SIDE * sinf(a) });
    } else if (sys.moveRight > 0) {
        PhysicsAddForce(tb, (Vector2){ L3_SIDE * cosf(tb->orient), L3_SIDE * sinf(tb->orient) });
    }
}

static void l3_landed(void) { sys.landed = true; }

static void l3_crash(void) {
    PhysicsBody tb = (PhysicsBody)tower.body;
    if (!tower.crashed) sys.score03 -= 5000;
    tower.fire = false;
    tower.crashed = true;
    StopSound(assets.fire);
    PlaySound(assets.crash);
    FxExplosion((Vector2){ tb->position.x, tb->position.y }, 2.0f);
}

// Lower-corner-vs-terrain contact test (replaces Box2D beginContact).
static bool TerrainContact(void) {
    PhysicsBody tb = (PhysicsBody)tower.body;
    float hw = tower.width / 2.0f, hh = tower.height / 2.0f;
    float c = cosf(tb->orient), s = sinf(tb->orient);
    // Bottom-left and bottom-right corners in world space.
    Vector2 corners[2] = {
        { tb->position.x + (-hw*c - hh*s), tb->position.y + (-hw*s + hh*c) },
        { tb->position.x + ( hw*c - hh*s), tb->position.y + ( hw*s + hh*c) },
    };
    for (int i = 0; i < 2; i++)
        if (corners[i].y >= TerrainY(corners[i].x)) return true;
    return false;
}

static void l3_update(float dt) {
    if (startFade > 0.0f) startFade -= dt / 0.5f;   // ~0.5s settle-in from black
    PhysicsStep();
    PhysicsBody tb = (PhysicsBody)tower.body;

    if (!tower.crashed && !tower.empty && !sys.landed) l3_input();
    EdgePush(tb->position.x, tb->position.y);
    EdgeTop(tb->position.x, tb->position.y);

    sys.moveGas   -= dt;
    sys.moveRight -= dt;
    sys.moveLeft  -= dt;

    // Input windows (keyboard + pointer zones). Rotation is read directly in
    // l3_input; thrust/side use held keys for continuous control.
    if (ActDown(ACT_THRUST)) sys.moveGas   = sys.moveGasTime;
    if (ActDown(ACT_RIGHT))  sys.moveRight = sys.moveRightTime;
    if (ActDown(ACT_LEFT))   sys.moveLeft  = sys.moveLeftTime;
    if (PointerPressed()) {
        float x = PointerX();
        if (x < sys.winWidth * 0.3f) sys.moveLeft = sys.moveLeftTime;
        else if (x > sys.winWidth * 0.7f) sys.moveRight = sys.moveRightTime;
        else sys.moveGas = sys.moveGasTime;
    }

    if (sys.score03 <= 0 || sys.landedTimer == 0) {
        if (sys.score03 <= 0) sys.score03 = 0;
        tower.empty = true;
        tower.fire = false;
    }

    // Contact handling (edge-detected, replaces beginContact/endContact).
    bool nowContact = TerrainContact();
    if (nowContact && !contact && !tower.crashed && !sys.winner) {
        if (!landingSpeedSet) { landingSpeed = tb->velocity.y; landingSpeedSet = true; }
        if (sys.debugMode) TraceLog(LOG_INFO, "L3 contact vy=%.3f orient=%.1fdeg",
                                    tb->velocity.y, tb->orient * RAD2DEG);
        if (tb->velocity.y < L3_LANDV) {
            l3_landed();
            FxDust((Vector2){ tb->position.x, tb->position.y + tower.height * 0.5f },
                   24, (Color){ 150, 115, 95, 255 });   // touchdown dust
        } else {
            l3_crash();
        }
        tb->enabled = false;   // freeze the lander where it touched down
        tb->velocity = (Vector2){ 0, 0 };
        tb->angularVelocity = 0;
    }
    contact = nowContact;

    if (!sys.winner) {
        if (sys.landed) {
            if (sys.landedTimer <= 0) sys.landedTimer = 0;
            else sys.landedTimer -= dt;
            if (sys.landedTimer == 0) {
                sys.winner = true;
                if (!tower.crashed) {
                    PlaySound(assets.landed);
                    FxBurst((Vector2){ tb->position.x, tb->position.y }, 44,
                            (Color){ 90, 230, 200, 255 }, 60, 320, 0.5f, 1.2f, 5, 120);
                }
                float deg = fabsf(tb->orient * RAD2DEG);
                while (deg > 360) deg -= 360;
                if (deg > 180) deg = 360 - deg;
                // Speed reward: the original used (100 - landingSpeed) with a
                // Box2D landing speed (px/s, ~tens-to-hundreds). physac's
                // velocity is in much smaller units, so that term barely moved
                // and every safe landing scored near the 4400 max. Map our
                // touchdown speed onto the same 0..100 range relative to the
                // safe-landing threshold so gentler landings still earn more.
                float speedReward = 100.0f * (1.0f - landingSpeed / L3_LANDV);
                if (speedReward < 0)   speedReward = 0;
                if (speedReward > 100) speedReward = 100;
                // Pad multiplier: where the lander's footprint touched down.
                int seg = SegmentAt(tb->position.x);
                padMult = PadMultiplier(seg);
                onPad   = (padMult > 1.0f);
                if (deg >= 45) sys.bonus = 0;
                else sys.bonus = (44 - deg) * speedReward * padMult;
                if (sys.bonus < 0) sys.bonus = 0;
                if (sys.bonus > 0) {
                    char pbuf[24];
                    snprintf(pbuf, sizeof(pbuf), "+%d", (int)ceilf(sys.bonus));
                    FxPopup((Vector2){ tb->position.x, tb->position.y - 30 }, pbuf,
                            (Color){ 120, 240, 210, 255 }, 44.0f);
                    if (onPad)
                        FxPopup((Vector2){ tb->position.x, tb->position.y - 78 }, "PAD!",
                                (Color){ 255, 220, 90, 255 }, 34.0f);
                }
            }
        }
        if (tower.fire && !tower.empty) {
            sys.score03 -= 200 * dt;
            AnimAdvance(&assets.fireball, 25.0f * dt);
            if (!IsSoundPlaying(assets.fire)) PlaySound(assets.fire);
            // Engine exhaust out the bottom (opposite the thrust direction).
            float a = tb->orient - 1.57f;
            Vector2 td = { cosf(a), sinf(a) };
            Vector2 emit = { tb->position.x - td.x * tower.height * 0.5f,
                             tb->position.y - td.y * tower.height * 0.5f };
            FxExhaust(emit, (Vector2){ -td.x * 300, -td.y * 300 },
                      (Color){ 255, 160, 40, 255 }, 7, 2);
        } else {
            PauseSound(assets.fire);
        }
    }

    // Advance / restart.
    if (sys.winner && !tower.crashed && (ActPressed(ACT_CONFIRM) || PointerPressed()))
        ChangeScreen(SCREEN_GOAL3);
    // A crash (immediately, or after the win timer if it tipped over) costs a
    // life and replays the level.
    if (tower.crashed && (ActPressed(ACT_CONFIRM) || PointerPressed()))
        NextLifeOrGameOver(SCREEN_LEVEL3);
}

static void DrawTerrainFill(float yOffset, Color col) {
    float bottom = sys.winHeight + 50;
    for (int i = 0; i < surfN - 1; i++) {
        Vector2 a = { surf[i].x,   surf[i].y + yOffset };
        Vector2 b = { surf[i+1].x, surf[i+1].y + yOffset };
        // Two triangles forming the quad down to the screen bottom.
        DrawTriangle((Vector2){ a.x, bottom }, b, a, col);
        DrawTriangle((Vector2){ a.x, bottom }, (Vector2){ b.x, bottom }, b, col);
    }
}

// Bright landing strips + posts on the flat pad segments.
static void DrawPads(void) {
    Color c = (Color){ 90, 230, 200, 255 };
    for (int i = 0; i < surfN - 1; i++) {
        if (!segPad[i]) continue;
        int x0 = (int)surf[i].x, x1 = (int)surf[i + 1].x, y = (int)surf[i].y;
        DrawRectangle(x0, y - 3, x1 - x0, 4, c);   // strip
        DrawRectangle(x0, y - 13, 3, 13, c);       // left post
        DrawRectangle(x1 - 3, y - 13, 3, 13, c);   // right post
    }
}

// Flight HUD (top-right, under the ships): descent / drift / tilt with a
// green-when-safe, red-when-dangerous cue + an overall go/no-go.
static void DrawLanderHUD(PhysicsBody tb) {
    const float fs = 26.0f, lh = fs + 4.0f;
    float x = sys.winWidth - 250.0f, y = 64.0f;
    Color safe = (Color){ 90, 230, 120, 255 };
    Color warn = (Color){ 255, 90, 60, 255 };
    Color cau  = (Color){ 255, 200, 40, 255 };

    float vdesc  = tb->velocity.y;            // + = descending
    float vdrift = fabsf(tb->velocity.x);
    float deg = fabsf(tb->orient * RAD2DEG);
    while (deg > 360) deg -= 360;
    if (deg > 180) deg = 360 - deg;

    char buf[48];
    snprintf(buf, sizeof buf, "DESCENT %4d", (int)(vdesc * 100));
    DrawTextEx(assets.screenFont, buf, (Vector2){ x, y }, fs, 1,
               vdesc < L3_LANDV ? safe : warn);
    snprintf(buf, sizeof buf, "DRIFT   %4d", (int)(vdrift * 100));
    DrawTextEx(assets.screenFont, buf, (Vector2){ x, y + lh }, fs, 1,
               vdrift < L3_LANDV ? safe : warn);
    snprintf(buf, sizeof buf, "TILT    %4d", (int)deg);
    DrawTextEx(assets.screenFont, buf, (Vector2){ x, y + 2*lh }, fs, 1,
               deg < 15.0f ? safe : (deg < 45.0f ? cau : warn));

    bool ok = (vdesc < L3_LANDV) && (vdrift < L3_LANDV) && (deg < 45.0f);
    DrawTextEx(assets.screenFont, ok ? "SAFE TO LAND" : "UNSAFE",
               (Vector2){ x, y + 3*lh + 4 }, fs, 1, ok ? safe : warn);
}

static void l3_draw(void) {
    PhysicsBody tb = (PhysicsBody)tower.body;

    DrawTexturePro(assets.bgMars,
        (Rectangle){ 0, 0, (float)assets.bgMars.width, (float)assets.bgMars.height },
        (Rectangle){ 0, 0, sys.winWidth, sys.winHeight }, (Vector2){ 0, 0 }, 0.0f,
        (Color){ (unsigned char)(sys.BGcolorR*255), (unsigned char)(sys.BGcolorG*255),
                 (unsigned char)(sys.BGcolorB*255), 255 });

    DrawTerrainFill(sys.winHeight * 0.04f, (Color){ 161, 125, 107, 255 }); // foreground ridge
    DrawTerrainFill(0.0f,                  (Color){ 135, 100, 82, 255 });  // main ground
    DrawPads();

    // Lander.
    DrawTexturePro(tower.image,
        (Rectangle){ 0, 0, tower.width, tower.height },
        (Rectangle){ tb->position.x, tb->position.y, tower.width, tower.height },
        (Vector2){ tower.width / 2.0f, tower.height / 2.0f }, tb->orient * RAD2DEG, WHITE);

    if (tower.fire) {
        DrawTexturePro(assets.fireball.tex, AnimFrame(&assets.fireball),
            (Rectangle){ tb->position.x, tb->position.y, 107, 147 },
            (Vector2){ 53, -60 }, tb->orient * RAD2DEG, Fade(WHITE, 0.7f));
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%d", (int)sys.score03);
    DrawTextEx(assets.scoreFont, buf, (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.1f },
               (float)assets.scoreFont.baseSize, 2, (Color){ 255, 0, 0, 255 });

    Color red = (Color){ 255, 0, 0, 255 };
    // Flight HUD only while actually flying.
    if (!sys.winner && !tower.crashed) DrawLanderHUD(tb);

    if (sys.winner && !tower.crashed) {
        DrawTextEx(assets.screenFont, "Landed!",
                   (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.3f },
                   (float)assets.screenFont.baseSize, 2, red);
        if (onPad) {
            snprintf(buf, sizeof(buf), "Landing Pad x%.1f!", padMult);
            DrawTextEx(assets.screenFont, buf,
                       (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.42f },
                       (float)assets.screenFont.baseSize, 2, (Color){ 90, 230, 200, 255 });
        }
        snprintf(buf, sizeof(buf), "Landing Bonus %d", (int)ceilf(sys.bonus));
        DrawTextEx(assets.screenFont, buf,
                   (Vector2){ sys.winWidth * 0.1f, sys.winHeight * 0.55f },
                   (float)assets.screenFont.baseSize, 2, red);
    }
    if (tower.crashed)
        DrawWrappedText("Crashed!\n\nPress to play again.", assets.screenFont,
                        sys.winWidth * 0.1f, sys.winHeight * 0.3f, sys.winWidth * 0.8f, ALIGN_LEFT, red);

    DrawLives();

    if (startFade > 0.0f)   // settle-in fade from black at level start
        DrawRectangle(0, 0, (int)sys.winWidth, (int)sys.winHeight, Fade(BLACK, Smooth01(startFade)));
}

static void l3_leave(void) {
    PhysicsReset();
    tower.body = NULL;
}

const Screen *Level3Screen(void) {
    static const Screen s = { l3_enter, l3_update, l3_draw, l3_leave };
    return &s;
}
