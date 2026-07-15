// main.c - entry point, global state, shared helpers, and the game loop.
#include "game.h"
#include "assets.h"
#include "physics.h"
#include "input.h"
#include "fx.h"
#include "crt.h"        // engine CRT post-process (lightly-salted-engine)
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

System sys;
Tower  tower;

// Shared online-leaderboard client (see game.h). Initialised in main().
LeaderboardData g_lb;

// CRT post-process (engine/crt). Subtle retro pass, on by default; F1 toggles it,
// MARTIN_CRT=0 forces it off. Shader differs by platform (desktop 330 / web ES 100).
#ifdef PLATFORM_WEB
  #define CRT_SHADER_PATH "assets/shaders/crt.web.fs"
#else
  #define CRT_SHADER_PATH "assets/shaders/crt.fs"
#endif
static CRTEffect g_crt;
static bool g_crtReady = false, g_crtOn = false;

// Music volume glides toward g_musicTarget instead of snapping (see UpdateGlobalAudio).
static float g_musicVol = 1.0f, g_musicTarget = 1.0f;
void SetMusicTarget(float v) { g_musicTarget = v; }

// ----------------------------------------------------------------------------
// Shared helpers
// ----------------------------------------------------------------------------

// Ported fade(): ramp alpha toward 1 at `speed` per second.
float FadeIn(float dt, float a, float speed) {
    if (a < 1.0f) a += speed * dt;
    if (a > 1.0f) a = 1.0f;
    return a;
}

// Smoothstep easing of a 0..1 value (3t^2 - 2t^3): eased-in-and-out feel for fades.
float Smooth01(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

// Clears the per-attempt flags a fresh launch/landing needs (shared by
// ResetGame and the retry path so a replayed level starts clean).
static void ClearAttemptState(void) {
    sys.landed  = false;
    sys.landedTimer = 2.0f;
    sys.crashed = false;
    sys.winner  = false;
    sys.bonus   = 0;
    tower.fire    = false;
    tower.empty   = false;
    tower.crashed = false;
}

// Crash handling: spend a life. With lives left, replay the level that was
// being played (its enter() rebuilds local state, and each level recomputes its
// working score from the untouched upstream score, so the attempt restarts
// cleanly). Out of lives -> full reset back to the start.
void NextLifeOrGameOver(GameScreen retryScreen) {
    sys.lives -= 1;
    if (sys.lives > 0) {
        ClearAttemptState();
        // Level 1 owns score01 (its fuel), so a retry must start it empty;
        // levels 2/3 recompute their score from the upstream value in enter().
        if (retryScreen == SCREEN_LEVEL1) sys.score01 = 0;
        ChangeScreen(retryScreen);
    } else {
        sys.lives = START_LIVES;
        ResetGame();
        sys.score01 = 0;
        ChangeScreen(SCREEN_INTRO02);
    }
}

// Ported reset(): carries score forward and clears per-run state.
void ResetGame(void) {
    sys.lives   = START_LIVES;
    sys.BGScale = 1.0f;
    sys.BGcolorR = sys.BGcolorG = sys.BGcolorB = 1.0f;
    sys.score01 = (float)((int)sys.score03 + (int)sys.bonus);
    sys.score02 = 0;
    sys.score03 = 0;
    sys.score   = 0;
    sys.landed  = false;
    sys.landedTimer = 2.0f;
    sys.launch  = LAUNCH_LEFT;
    sys.crashed = false;
    sys.winner  = false;
    sys.timer   = 0;
    sys.bonus   = 0;
    tower.strengthTorque = 2000.0f;
    tower.strengthMain   = 2000.0f;
    tower.strengthSide   = 300.0f;
    tower.fire = false;
    tower.empty = false;
    tower.crashed = false;
}

// Ported edge(): push the tower back toward center if it nears a screen edge.
void EdgePush(float x, float y) {
    PhysicsBody b = (PhysicsBody)tower.body;
    if (!b) return;
    if (x < sys.winWidth * 0.1f) {
        PhysicsAddForce(b, (Vector2){ 100, 0 });
        sys.BGcolorG = sys.BGcolorB = 0.25f;
        if (x < sys.winWidth * 0.05f) {
            PhysicsAddForce(b, (Vector2){ 1000, 0 });
            sys.BGcolorG = sys.BGcolorB = 0.01f;
        }
    } else if (x > sys.winWidth * 0.9f) {
        PhysicsAddForce(b, (Vector2){ -100, 0 });
        sys.BGcolorG = sys.BGcolorB = 0.25f;
        if (x > sys.winWidth * 0.95f) {
            PhysicsAddForce(b, (Vector2){ -1000, 0 });
            sys.BGcolorG = sys.BGcolorB = 0.01f;
        }
    } else {
        sys.BGcolorG = sys.BGcolorB = 1.0f;
    }
}

// Ported edgeTop().
void EdgeTop(float x, float y) {
    PhysicsBody b = (PhysicsBody)tower.body;
    if (!b) return;
    if (y < -sys.winHeight * 0.3f) {
        PhysicsAddForce(b, (Vector2){ 0, 10000 });
        sys.BGcolorG = sys.BGcolorB = 0.01f;
    }
}

// High score persistence (plain int in a file beside the working dir).
#define HIGHSCORE_FILE "highscore.dat"

int LoadHighScore(void) {
    int v = 0;
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    if (f) { if (fscanf(f, "%d", &v) != 1) v = 0; fclose(f); }
    return v;
}

bool SubmitHighScore(int finalScore) {
    if (finalScore <= sys.highScore) return false;
    sys.highScore = finalScore;
    FILE *f = fopen(HIGHSCORE_FILE, "w");
    if (f) { fprintf(f, "%d\n", finalScore); fclose(f); }
    return true;
}

// ----------------------------------------------------------------------------
// Online leaderboard config + player-name persistence.
// ----------------------------------------------------------------------------

// Server connection + game identity for every Submit/Fetch/Validate call. The
// shared server partitions all boards by `game`, so "martin" is this game's slice.
LeaderboardConfig MartinLBConfig(void) {
    LeaderboardConfig c = { 0 };
    snprintf(c.url,   sizeof(c.url),   "%s", "https://scores.lightly-salted.com/api");
    snprintf(c.game,  sizeof(c.game),  "%s", "martin");
    snprintf(c.build, sizeof(c.build), "%s", "0.16.0");
    snprintf(c.platform, sizeof(c.platform), "%s", "desktop-linux");
    c.install_id[0] = '\0';  // per-install analytics id (optional) — omitted
    c.playtime = -1.0;       // omit per-run telemetry
    c.runs     = -1.0;
    return c;
}

// Player name lives in a plain-text file beside the working dir (one line).
void LoadPlayerName(void) {
    sys.playerName[0] = '\0';
    FILE *f = fopen(PLAYERNAME_FILE, "r");
    if (!f) return;
    if (fgets(sys.playerName, sizeof(sys.playerName), f)) {
        size_t len = strlen(sys.playerName);
        while (len > 0 && (sys.playerName[len-1] == '\n' || sys.playerName[len-1] == '\r'))
            sys.playerName[--len] = '\0';
    }
    fclose(f);
}

void SavePlayerName(void) {
    FILE *f = fopen(PLAYERNAME_FILE, "w");
    if (!f) return;
    fputs(sys.playerName, f);
    fclose(f);
}

// Shared slow-drifting, twinkling starfield used as a living backdrop behind the
// otherwise-black text / goal / stage screens.
void DrawStarfield(void) {
    static bool init = false;
    static Vector2 s[120];
    static float   b[120], sz[120];
    if (!init) {
        for (int i = 0; i < 120; i++) {
            s[i]  = (Vector2){ (float)GetRandomValue(0, GAME_WIDTH),
                               (float)GetRandomValue(0, GAME_HEIGHT) };
            b[i]  = (float)GetRandomValue(30, 100) / 100.0f;
            sz[i] = (float)GetRandomValue(1, 3);
        }
        init = true;
    }
    float t = (float)GetTime();
    for (int i = 0; i < 120; i++) {
        float y  = fmodf(s[i].y + t * (8.0f + b[i] * 16.0f), (float)GAME_HEIGHT);
        float tw = 0.65f + 0.35f * sinf(t * 1.7f + i * 0.6f);   // twinkle
        DrawRectangle((int)s[i].x, (int)y, (int)sz[i], (int)sz[i],
                      Fade(WHITE, b[i] * tw * 0.85f));
    }
}

// Shared HUD: remaining ships, drawn top-right as small tower icons.
void DrawLives(void) {
    const float s = 0.30f;                         // icon scale
    float iw = tower.width * s, ih = tower.height * s;
    for (int i = 0; i < sys.lives; i++) {
        float x = sys.winWidth - 12.0f - (i + 1) * (iw + 6.0f);
        DrawTexturePro(tower.image,
            (Rectangle){ 0, 0, tower.width, tower.height },
            (Rectangle){ x, 12.0f, iw, ih },
            (Vector2){ 0, 0 }, 0.0f, Fade(WHITE, 0.85f));
    }
}

// Word-wrapped, alpha-aware text (replaces love.graphics.printf with alignment).
void DrawWrappedText(const char *text, Font font, float x, float y, float wrapW,
                     TextAlign align, Color color) {
    const float fontSize   = (float)font.baseSize;
    const float spacing    = fontSize / 16.0f;
    const float lineHeight = fontSize * 1.15f;

    char line[2048] = {0};
    char word[512]  = {0};
    int  lineLen = 0, wordLen = 0;
    float cy = y;

    // Flush helper draws the current line with the requested alignment.
    #define FLUSH_LINE() do {                                                  \
        Vector2 m = MeasureTextEx(font, line, fontSize, spacing);              \
        float lx = x;                                                          \
        if (align == ALIGN_CENTER) lx = x + (wrapW - m.x) / 2.0f;              \
        else if (align == ALIGN_RIGHT) lx = x + (wrapW - m.x);                 \
        DrawTextEx(font, line, (Vector2){ lx, cy }, fontSize, spacing, color); \
        cy += lineHeight; line[0] = 0; lineLen = 0;                            \
    } while (0)

    for (const char *p = text; ; p++) {
        char c = *p;
        if (c == ' ' || c == '\n' || c == '\0') {
            if (wordLen > 0) {
                word[wordLen] = 0;
                // Would the word fit on the current line?
                char trial[2048];
                if (lineLen > 0) snprintf(trial, sizeof(trial), "%s %s", line, word);
                else             snprintf(trial, sizeof(trial), "%s", word);
                Vector2 m = MeasureTextEx(font, trial, fontSize, spacing);
                if (m.x > wrapW && lineLen > 0) {
                    FLUSH_LINE();
                    snprintf(line, sizeof(line), "%s", word);
                } else {
                    snprintf(line, sizeof(line), "%s", trial);
                }
                lineLen = (int)strlen(line);
                wordLen = 0;
            }
            if (c == '\n') FLUSH_LINE();          // hard break (also handles blank lines)
            if (c == '\0') { FLUSH_LINE(); break; }
        } else if (wordLen < (int)sizeof(word) - 1) {
            word[wordLen++] = c;
        }
    }
    #undef FLUSH_LINE
}

// ----------------------------------------------------------------------------
// Global per-frame concerns shared by every screen in the original:
// background music streaming + the 'm' play/pause toggle.
// ----------------------------------------------------------------------------
static void UpdateGlobalAudio(void) {
    UpdateMusicStream(assets.mainTheme);
    // Glide the music volume toward its target so screen changes don't snap.
    float k = GetFrameTime() * 4.0f; if (k > 1.0f) k = 1.0f;
    g_musicVol += (g_musicTarget - g_musicVol) * k;
    SetMusicVolume(assets.mainTheme, g_musicVol);
    if (IsKeyPressed(KEY_M)) {
        if (IsMusicStreamPlaying(assets.mainTheme)) PauseMusicStream(assets.mainTheme);
        else                                        ResumeMusicStream(assets.mainTheme);
    }
    if (IsKeyPressed(KEY_F1) && g_crtReady) g_crtOn = !g_crtOn;   // toggle the CRT pass
}

static GameScreen ScreenFromName(const char *n) {
    if (!strcmp(n, "menu"))    return SCREEN_MENU;
    if (!strcmp(n, "intro02")) return SCREEN_INTRO02;
    if (!strcmp(n, "level1"))  return SCREEN_LEVEL1;
    if (!strcmp(n, "goal1"))   return SCREEN_GOAL1;
    if (!strcmp(n, "level2"))  return SCREEN_LEVEL2;
    if (!strcmp(n, "goal2"))   return SCREEN_GOAL2;
    if (!strcmp(n, "level3"))  return SCREEN_LEVEL3;
    if (!strcmp(n, "goal3"))   return SCREEN_GOAL3;
    if (!strcmp(n, "stage"))   return SCREEN_STAGE;
    if (!strcmp(n, "nameentry"))   return SCREEN_NAME_ENTRY;
    if (!strcmp(n, "leaderboard"))  return SCREEN_LEADERBOARD;
    return SCREEN_INTRO;
}

// Draw one full frame of the game (scene through the shake camera, particles,
// fade overlay, debug FPS). Rendered either straight to the backbuffer or into
// the CRT target, so both paths share exactly the same scene composition.
static void DrawSceneFrame(void) {
    ClearBackground(BLACK);
    GameScreen cur = CurrentScreen();
    Camera2D shakeCam = { 0 };
    shakeCam.zoom = 1.0f;
    shakeCam.offset = FxShakeOffset();
    BeginMode2D(shakeCam);
        if (screens[cur] && screens[cur]->draw) screens[cur]->draw();
        FxDraw();
    EndMode2D();
    TransitionDraw();                 // fade overlay sits above everything
    if (sys.debugMode) DrawFPS(10, 10);
}

int main(int argc, char **argv) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "Send Martin Tower to Mars!");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE);

    // Global state defaults (ported from objects.lua).
    sys.winWidth = GAME_WIDTH;
    sys.winHeight = GAME_HEIGHT;
    sys.scaling = (float)GetScreenWidth() / GAME_WIDTH;
    sys.scalingH = (float)GetScreenHeight() / GAME_HEIGHT;
    sys.level = 1;
    sys.lives = START_LIVES;
    sys.highScore = LoadHighScore();
    LoadPlayerName();
    sys.landedTimer = 2.0f;
    sys.BGScale = 1.0f;
    sys.BGcolorR = sys.BGcolorG = sys.BGcolorB = 1.0f;
    sys.launch = LAUNCH_LEFT;
    sys.moveLeftTime = sys.moveRightTime = sys.moveGasTime = 0.4f;
    sys.version = "0.16.0 - Raylib";

    tower.strengthTorque = 2000.0f;
    tower.strengthMain   = 1600.0f;
    tower.strengthSide   = 300.0f;

    LoadAssets();
    Image iconImg = LoadImageFromTexture(assets.icon);
    SetWindowIcon(iconImg);
    UnloadImage(iconImg);

    PhysicsInit();
    FxInit();
    RegisterScreens();

    // CRT post-process (engine): render the whole game into a fixed-res target and
    // blit it back through a subtle scanline/curvature/vignette shader. On by default
    // (F1 toggles; MARTIN_CRT=0 forces off). Fails closed -> plain rendering if the
    // shader can't compile/load.
    g_crtReady = CRT_Init(&g_crt, CRT_SHADER_PATH, GAME_WIDTH, GAME_HEIGHT);
    if (g_crtReady) {
        CRT_SetIntensity(&g_crt, 0.9f);
        CRT_SetScanlineStrength(&g_crt, 0.15f);
        CRT_SetCurvature(&g_crt, 0.08f);
        CRT_SetVignette(&g_crt, 0.22f);
        CRT_SetBloomStrength(&g_crt, 0.22f);
        CRT_SetStaticStrength(&g_crt, 0.0f);
        const char *crtEnv = getenv("MARTIN_CRT");
        g_crtOn = !(crtEnv && crtEnv[0] == '0');
    }

    // Online leaderboard: init, load the CA bundle for HTTPS, then fire a one-off
    // connectivity probe so on/offline is known by the time a board is shown.
    Leaderboard_Init(&g_lb);
    char *caPem = LoadFileText("assets/cacert.pem");
    if (caPem) { Leaderboard_SetCA(&g_lb, caPem); UnloadFileText(caPem); }
    LeaderboardConfig lbProbe = MartinLBConfig();
    Leaderboard_Probe(&g_lb, &lbProbe);

    // Scripted demo input (verification only); empty/NULL = normal play.
    InputInitDemo(getenv("MARTIN_DEMO"));

    // Capture mode (verification): MARTIN_CAP="screen:delaySeconds[:out.png]".
    // Jumps to a screen, runs for delaySeconds, screenshots, then quits.
    bool       capMode   = false;
    float      capDelay  = 2.0f, capElapsed = 0.0f;
    char       capOut[256] = "cap.png";
    const char *capEnv = getenv("MARTIN_CAP");

    // Debug entry: `./game debug <level>` jumps straight into a level.
    sys.debugMode = (argc >= 2 && strcmp(argv[1], "debug") == 0) || (capEnv != NULL);

    if (capEnv) {
        char buf[300]; snprintf(buf, sizeof(buf), "%s", capEnv);
        char *name = strtok(buf, ":");
        char *delay = strtok(NULL, ":");
        char *out = strtok(NULL, ":");
        capMode = true;
        if (delay) capDelay = (float)atof(delay);
        if (out)   snprintf(capOut, sizeof(capOut), "%s", out);
        sys.score01 = sys.score02 = sys.score03 = 9000;
        ChangeScreen(ScreenFromName(name ? name : "intro"));
    } else if (sys.debugMode) {
        sys.score02 = 9000;
        sys.score03 = 9000;
        const char *lvl = (argc >= 3) ? argv[2] : "";
        if      (strcmp(lvl, "01") == 0) ChangeScreen(SCREEN_LEVEL1);
        else if (strcmp(lvl, "02") == 0) ChangeScreen(SCREEN_LEVEL2);
        else if (strcmp(lvl, "03") == 0) ChangeScreen(SCREEN_LEVEL3);
        else                             ChangeScreen(SCREEN_INTRO);
    } else {
        ChangeScreen(SCREEN_INTRO);
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        UpdateGlobalAudio();
        Leaderboard_Update(&g_lb);   // reap finished fetch/submit/validate threads
        GameScreen cur = CurrentScreen();
        if (!TransitionFreezing() && screens[cur] && screens[cur]->update)
            screens[cur]->update(dt);
        TransitionUpdate(dt);
        FxUpdate(dt);

        if (g_crtReady) {
            // Render the scene into the CRT target, then blit it to the window
            // (shaded when on, plain scaled blit when toggled off).
            CRT_BeginScene(&g_crt);
                DrawSceneFrame();
            CRT_EndScene(&g_crt);
            BeginDrawing();
                ClearBackground(BLACK);
                Rectangle full = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
                if (g_crtOn) CRT_Draw(&g_crt, full);
                else         CRT_Present(&g_crt, full);
            EndDrawing();
        } else {
            BeginDrawing();
                DrawSceneFrame();
            EndDrawing();
        }

        if (capMode) {
            capElapsed += dt;
            if (capElapsed >= capDelay) { TakeScreenshot(capOut); break; }
        }
    }

    Leaderboard_Destroy(&g_lb);
    if (g_crtReady) CRT_Unload(&g_crt);
    PhysicsClose();
    UnloadAssets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
