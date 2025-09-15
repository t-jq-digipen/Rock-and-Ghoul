#include "MovementController.h"

MovementController::MovementController(ControlScheme scheme, float speed)
    : controlScheme(scheme), speed(speed) {}

void MovementController::Update(float& x, float& y, float dt) {
    float dx = 0.0f, dy = 0.0f;

    if (controlScheme == ControlScheme::WASD) {
        if (Input::IsKeyHeld(GLFW_KEY_W)) dy += speed;
        if (Input::IsKeyHeld(GLFW_KEY_S)) dy -= speed;
        if (Input::IsKeyHeld(GLFW_KEY_A)) dx -= speed;
        if (Input::IsKeyHeld(GLFW_KEY_D)) dx += speed;
    }
    else if (controlScheme == ControlScheme::Arrows) {
        if (Input::IsKeyHeld(GLFW_KEY_UP))    dy += speed;
        if (Input::IsKeyHeld(GLFW_KEY_DOWN))  dy -= speed;
        if (Input::IsKeyHeld(GLFW_KEY_LEFT))  dx -= speed;
        if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) dx += speed;
    }

    x += dx * dt;
    y += dy * dt;
}
