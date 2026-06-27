// input.c - action mapping over keyboard / pointer / scripted demo.
#include "input.h"
#include <string.h>

static bool demo[ACT_COUNT];

void InputInitDemo(const char *spec) {
    for (int i = 0; i < ACT_COUNT; i++) demo[i] = false;
    if (!spec || !spec[0]) return;
    if (strstr(spec, "left"))   demo[ACT_LEFT]   = true;
    if (strstr(spec, "right"))  demo[ACT_RIGHT]  = true;
    if (strstr(spec, "thrust")) demo[ACT_THRUST] = true;
    if (strstr(spec, "rotl"))   demo[ACT_ROTL]   = true;
    if (strstr(spec, "rotr"))   demo[ACT_ROTR]   = true;
    if (strstr(spec, "confirm"))demo[ACT_CONFIRM]= true;
}

static bool KeyDownFor(Action a) {
    switch (a) {
        case ACT_LEFT:    return IsKeyDown(KEY_LEFT);
        case ACT_RIGHT:   return IsKeyDown(KEY_RIGHT);
        case ACT_THRUST:  return IsKeyDown(KEY_UP);
        case ACT_ROTL:    return IsKeyDown(KEY_A);
        case ACT_ROTR:    return IsKeyDown(KEY_D);
        case ACT_CONFIRM: return IsKeyDown(KEY_SPACE);
        default:          return false;
    }
}

static bool KeyPressedFor(Action a) {
    switch (a) {
        case ACT_LEFT:    return IsKeyPressed(KEY_LEFT);
        case ACT_RIGHT:   return IsKeyPressed(KEY_RIGHT);
        case ACT_THRUST:  return IsKeyPressed(KEY_UP);
        case ACT_ROTL:    return IsKeyPressed(KEY_A);
        case ACT_ROTR:    return IsKeyPressed(KEY_D);
        case ACT_CONFIRM: return IsKeyPressed(KEY_SPACE);
        default:          return false;
    }
}

bool ActDown(Action a)    { return demo[a] || KeyDownFor(a); }
bool ActPressed(Action a) { return demo[a] || KeyPressedFor(a); }

bool  PointerPressed(void) { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT); }
bool  PointerDown(void)    { return IsMouseButtonDown(MOUSE_BUTTON_LEFT); }
float PointerX(void)       { return (float)GetMouseX(); }
float PointerY(void)       { return (float)GetMouseY(); }
