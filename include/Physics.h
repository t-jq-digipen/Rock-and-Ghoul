// physics.h
#pragma once

// Physics constants
namespace Physics {
    const float GRAVITY = -800.0f;           // pixels per second squared
    const float JUMP_VELOCITY = 400.0f;      // initial jump speed
    const float TERMINAL_VELOCITY = -600.0f; // maximum falling speed
}

// TryJump is to check jump state, ApplyJumpPhysics is to actually have player jump and fall
bool TryJump(bool jumpPressed, bool inJumpState, float& velocityY, bool& jumpState, float currentY, float& jumpStartY);
void ApplyJumpPhysics(bool inJumpState, float& velocityY, float& nextY, float currentY, float jumpStartY, bool& jumpState, float dt);