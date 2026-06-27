// input.h - centralized input so gameplay reads intent, not raw devices.
// Maps keyboard, pointer/touch zones, and an optional scripted demo (for
// automated verification) onto a small set of game actions.
#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    ACT_LEFT = 0,   // steer / fill left
    ACT_RIGHT,      // steer / fill right
    ACT_THRUST,     // main engine (up / center tap)
    ACT_ROTL,       // rotate counter-clockwise (a)
    ACT_ROTR,       // rotate clockwise (d)
    ACT_CONFIRM,    // advance / restart (space / tap)
    ACT_COUNT
} Action;

// Parse an optional scripted-demo spec (comma list of action names, e.g.
// "thrust,rotl"); pass NULL/"" for normal play. Demo actions report as both
// held and pressed every frame.
void InputInitDemo(const char *spec);

bool ActDown(Action a);     // continuously active this frame
bool ActPressed(Action a);  // newly activated this frame

// Pointer helpers for screen-region taps (desktop mouse + mobile touch).
bool PointerPressed(void);   // newly pressed this frame
bool PointerDown(void);      // held down this frame (hold-to-thrust)
float PointerX(void);
float PointerY(void);

#endif // INPUT_H
