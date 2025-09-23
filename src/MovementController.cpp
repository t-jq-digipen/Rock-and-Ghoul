#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
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
    else if (controlScheme == ControlScheme::IJKL) {
        if (Input::IsKeyHeld(GLFW_KEY_I)) dy += speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_K)) dy -= speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_J)) dx -= speed * dt;
        if (Input::IsKeyHeld(GLFW_KEY_L)) dx += speed * dt;
    }

    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        double mx = Input::GetMouseX();
        double my = GLApp::height - Input::GetMouseY();
    }
}



