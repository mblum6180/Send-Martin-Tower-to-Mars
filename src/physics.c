// physics.c - physac implementation lives here (the single PHYSAC_IMPLEMENTATION TU).
#define PHYSAC_IMPLEMENTATION
#include "physics.h"

void PhysicsInit(void)  { InitPhysics(); }
void PhysicsClose(void) { ClosePhysics(); }
void PhysicsStep(void)  { UpdatePhysics(); }
void PhysicsReset(void) { ResetPhysics(); }
void PhysicsSetGravity(float gx, float gy) { SetPhysicsGravity(gx, gy); }
