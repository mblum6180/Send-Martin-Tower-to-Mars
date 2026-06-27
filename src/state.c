// state.c - screen registry + fade-through-black transitions (replaces HUMP).
#include "game.h"

const Screen *screens[SCREEN_COUNT];
static GameScreen current = SCREEN_INTRO;

// A screen change fades the frame out to black, swaps screens at full black,
// then fades back in. ChangeScreen() requests it; the main loop drives it via
// TransitionUpdate()/TransitionDraw().
typedef enum { TR_NONE, TR_OUT, TR_IN } TrState;
static TrState    trState = TR_NONE;
static float      trAlpha = 0.0f;          // black overlay alpha (0..1)
static GameScreen trTarget;
static bool       started = false;         // first switch is immediate (+ fade in)
static const float TR_SPEED = 4.5f;        // ~0.22s per fade phase

void RegisterScreens(void) {
    screens[SCREEN_INTRO]   = IntroScreen();
    screens[SCREEN_MENU]    = MenuScreen();
    screens[SCREEN_INTRO02] = Intro02Screen();
    screens[SCREEN_LEVEL1]  = Level1Screen();
    screens[SCREEN_GOAL1]   = Goal1Screen();
    screens[SCREEN_LEVEL2]  = Level2Screen();
    screens[SCREEN_GOAL2]   = Goal2Screen();
    screens[SCREEN_LEVEL3]  = Level3Screen();
    screens[SCREEN_GOAL3]   = Goal3Screen();
    screens[SCREEN_STAGE]   = StageScreen();
}

GameScreen CurrentScreen(void) { return current; }

static void DoSwitch(GameScreen next) {
    if (screens[current] && screens[current]->leave) screens[current]->leave();
    current = next;
    if (screens[current] && screens[current]->enter) screens[current]->enter();
}

void ChangeScreen(GameScreen next) {
    if (!started) {                 // startup: enter immediately, fade up from black
        started = true;
        DoSwitch(next);
        trState = TR_IN;
        trAlpha = 1.0f;
        return;
    }
    if (trState != TR_NONE) return; // ignore requests while already transitioning
    trTarget = next;
    trState  = TR_OUT;
    trAlpha  = 0.0f;
}

void TransitionUpdate(float dt) {
    if (trState == TR_OUT) {
        trAlpha += TR_SPEED * dt;
        if (trAlpha >= 1.0f) { trAlpha = 1.0f; DoSwitch(trTarget); trState = TR_IN; }
    } else if (trState == TR_IN) {
        trAlpha -= TR_SPEED * dt;
        if (trAlpha <= 0.0f) { trAlpha = 0.0f; trState = TR_NONE; }
    }
}

void TransitionDraw(void) {
    if (trAlpha > 0.0f)
        DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, Fade(BLACK, trAlpha));
}

// True while fading out: the outgoing screen is frozen (not updated) so it can't
// run on partially-reset state during the fade.
bool TransitionFreezing(void) { return trState == TR_OUT; }
