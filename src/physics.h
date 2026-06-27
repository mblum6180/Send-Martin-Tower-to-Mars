// physics.h - thin wrapper over physac (single global world).
// Including this also exposes the physac API (declarations only) for level code.
#ifndef PHYSICS_H
#define PHYSICS_H

#include "raylib.h"
#include "../vendor/physac.h"

void PhysicsInit(void);              // once at startup
void PhysicsClose(void);             // once at shutdown
void PhysicsStep(void);              // each frame
void PhysicsReset(void);             // destroy all bodies (on level leave)
void PhysicsSetGravity(float gx, float gy);

#endif // PHYSICS_H
