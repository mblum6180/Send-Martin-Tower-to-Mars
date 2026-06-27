// game.h - shared types, globals, and prototypes for Send Martin Tower to Mars!
// C + raylib port of the original LOVE/Lua game.
#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>

// Logical design resolution (everything is authored against this and scaled to the window).
#define GAME_WIDTH  1280
#define GAME_HEIGHT 720

// ----------------------------------------------------------------------------
// Game screens (replaces HUMP gamestate). Order matches the original flow.
// ----------------------------------------------------------------------------
typedef enum {
    SCREEN_INTRO = 0,
    SCREEN_MENU,
    SCREEN_INTRO02,
    SCREEN_LEVEL1,
    SCREEN_GOAL1,
    SCREEN_LEVEL2,
    SCREEN_GOAL2,
    SCREEN_LEVEL3,
    SCREEN_GOAL3,
    SCREEN_STAGE,
    SCREEN_COUNT
} GameScreen;

// A screen is a set of lifecycle callbacks (any may be NULL).
typedef struct {
    void (*enter)(void);
    void (*update)(float dt);
    void (*draw)(void);
    void (*leave)(void);
} Screen;

// Toggle used by the Level 1 fuel-fill rhythm (alternate left/right).
typedef enum { LAUNCH_LEFT = 0, LAUNCH_RIGHT = 1 } LaunchSide;

// ----------------------------------------------------------------------------
// Tower: the player object. Holds tuning + the active physac body (void* so this
// header stays free of physac).
// ----------------------------------------------------------------------------
typedef struct {
    Texture2D image;
    float width, height;
    float strengthTorque;
    float strengthMain;
    float strengthSide;
    bool fire;
    bool empty;
    bool crashed;
    void *body;        // PhysicsBody, owned by the active level
} Tower;

// ----------------------------------------------------------------------------
// System: global game state (ported from objects.lua `sys`).
// ----------------------------------------------------------------------------
typedef struct {
    float winWidth, winHeight;   // logical size (GAME_WIDTH/HEIGHT)
    float scaling, scalingH;     // window/logical scale factors

    int   level;                 // difficulty round, increments after a full loop
    int   lives;                 // remaining attempts before a full reset to the start
    int   highScore;             // best final score, persisted across runs

    float score01, score02, score03, score, bonus;

    bool  landed;
    float landedTimer;
    int   itemsDestroyed;

    LaunchSide launch;           // rhythm toggle (Level 1)
    bool  crashed;
    bool  winner;

    float timer;

    float BGScale;
    float BGcolorR, BGcolorG, BGcolorB;

    // Tap/hold movement windows (seconds remaining), ported from the original.
    float moveLeft,  moveLeftTime;
    float moveRight, moveRightTime;
    float moveGas,   moveGasTime;

    const char *version;
    bool  debugMode;
} System;

extern System sys;
extern Tower  tower;

// ----------------------------------------------------------------------------
// Screen registry / transitions (state.c)
// ----------------------------------------------------------------------------
extern const Screen *screens[SCREEN_COUNT];
void RegisterScreens(void);
void ChangeScreen(GameScreen next);
GameScreen CurrentScreen(void);
void TransitionUpdate(float dt);   // drive the fade-through-black between screens
void TransitionDraw(void);         // draw the black overlay (call last, unshaken)
bool TransitionFreezing(void);     // true while fading out (freeze outgoing screen)

// Screen registration (each screens/*.c exposes its definition).
const Screen *IntroScreen(void);
const Screen *MenuScreen(void);
const Screen *Intro02Screen(void);
const Screen *Level1Screen(void);
const Screen *Goal1Screen(void);
const Screen *Level2Screen(void);
const Screen *Goal2Screen(void);
const Screen *Level3Screen(void);
const Screen *Goal3Screen(void);
const Screen *StageScreen(void);

// ----------------------------------------------------------------------------
// Shared helpers (main.c)
// ----------------------------------------------------------------------------
float FadeIn(float dt, float a, float speed);   // ported fade(): ramp a -> 1
void  ResetGame(void);                            // ported reset()
void  EdgePush(float x, float y);                 // ported edge()
void  EdgeTop(float x, float y);                  // ported edgeTop()

// Crash handling: spend a life and replay `retryScreen`, or (out of lives) do a
// full reset back to the start. Levels call this instead of resetting directly.
void  NextLifeOrGameOver(GameScreen retryScreen);
void  DrawLives(void);                            // shared HUD: ships top-right
void  DrawStarfield(void);                        // ambient backdrop for text screens
#define START_LIVES 3

// High score: load once at startup into sys.highScore; submit a final score
// (updates + persists sys.highScore, returns true if it was a new best).
int   LoadHighScore(void);
bool  SubmitHighScore(int finalScore);

// Word-wrapped, alpha-aware text in the screen font (replaces love printf 'center').
typedef enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT } TextAlign;
void DrawWrappedText(const char *text, Font font, float x, float y, float wrapW,
                     TextAlign align, Color color);

#endif // GAME_H
