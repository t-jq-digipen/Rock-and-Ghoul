#include "physics.h"

// Handle jump input - returns true if jump works!!!!!
bool TryJump(bool jumpPressed, bool inJumpState, float& velocityY, bool& jumpState, float currentY, float& jumpStartY) {
    if (jumpPressed && !inJumpState) {
        velocityY = Physics::JUMP_VELOCITY;
        jumpState = true;
        jumpStartY = currentY;  // REMEMBER WHERE JUMP STARTED
        return true;
    }
    return false; // DONT MODIFY JUMPSTARTY IF ALREADY JUMPING
}

// Apply physics during jump state - modifies velocityY and handles jump completion
void ApplyJumpPhysics(bool inJumpState, float& velocityY, float& nextY, float currentY, float jumpStartY, bool& jumpState, float dt) {
    // only if jumping, apply physics
    if (inJumpState) {
        velocityY += Physics::GRAVITY * dt;

        // calculate new position
        nextY = currentY + velocityY * dt;

        // jump is over when fall to original position or fall below
        if (velocityY < 0 && nextY <= jumpStartY) {
            // return to free movement and set jumping state to false
            jumpState = false;
            velocityY = 0.0f;
            nextY = jumpStartY; // Y VALUE WHERE JUMP WAS INITIALLY DONE
        }
    }
}