#include "MovementController.h"

MovementController::MovementController(ControlScheme scheme, float speed)
    : controlScheme(scheme), speed(speed), dx(0.0f), dy(0.0f) {}

void MovementController::ComputeInput(float dt) {
    dx = dy = 0.0f;

    if (controlScheme == ControlScheme::WASD) {
        if (Input::IsKeyHeld(GLFW_KEY_W)) dy += speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_S)) dy -= speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_A)) dx -= speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_D)) dx += speed * dt;
    }
    else if (controlScheme == ControlScheme::Arrows) {
        if (Input::IsKeyHeld(GLFW_KEY_UP))    dy += speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_DOWN))  dy -= speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_LEFT))  dx -= speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) dx += speed * dt;
    }
}
