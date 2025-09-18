#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "MovementController.h"

MovementController::MovementController(ControlScheme scheme, float speed)
    : controlScheme(scheme), speed(speed), dx(0.0f), dy(0.0f) {}

void MovementController::ComputeInput(float dt) {
    dx = dy = 0.0f;

    if (controlScheme == ControlScheme::WASD) {
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_W) == GLFW_PRESS) dy += speed * dt;
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_S) == GLFW_PRESS) dy -= speed * dt;
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_A) == GLFW_PRESS) dx -= speed * dt;
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_D) == GLFW_PRESS) dx += speed * dt;
    }
    else if (controlScheme == ControlScheme::Arrows) {
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_UP) == GLFW_PRESS)    dy += speed * dt;
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_DOWN) == GLFW_PRESS)  dy -= speed * dt;
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_LEFT) == GLFW_PRESS)  dx -= speed * dt;
        if (glfwGetKey(GLApp::ptr_window, GLFW_KEY_RIGHT) == GLFW_PRESS) dx += speed * dt;
    }
}
