#include "input.h"
#include <iostream>

// Define static members
std::unordered_map<int, KeyState> Input::keys;
std::unordered_map<int, KeyState> Input::mouseButtons;
double Input::mouseX = 0.0;
double Input::mouseY = 0.0;
double Input::scrollX = 0.0;
double Input::scrollY = 0.0;

void Input::Update() {
    // Reset transient states (PRESSED/RELEASED -> HELD/RELEASED)
    for (auto& [key, state] : keys) {
        if (state == KeyState::PRESSED)
            state = KeyState::HELD;
        else if (state == KeyState::RELEASED)
            state = KeyState::RELEASED; // stays released
    }

    for (auto& [button, state] : mouseButtons) {
        if (state == KeyState::PRESSED)
            state = KeyState::HELD;
        else if (state == KeyState::RELEASED)
            state = KeyState::RELEASED;
    }

    // Reset scroll each frame so it only reflects last tick’s scroll
    scrollX = 0.0;
    scrollY = 0.0;
}

// -----------------------------
// Query Functions
// -----------------------------
bool Input::IsKeyPressed(int key) {
    return keys[key] == KeyState::PRESSED;
}

bool Input::IsKeyHeld(int key) {
    return keys[key] == KeyState::HELD;
}

bool Input::IsKeyReleased(int key) {
    return keys[key] == KeyState::RELEASED;
}

bool Input::IsMousePressed(int button) {
    return mouseButtons[button] == KeyState::PRESSED;
}

bool Input::IsMouseHeld(int button) {
    return mouseButtons[button] == KeyState::HELD;
}

bool Input::IsMouseReleased(int button) {
    return mouseButtons[button] == KeyState::RELEASED;
}

double Input::GetMouseX() { return mouseX; }
double Input::GetMouseY() { return mouseY; }
double Input::GetScrollX() { return scrollX; }
double Input::GetScrollY() { return scrollY; }

// -----------------------------
// GLFW Callbacks
// -----------------------------
void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        keys[key] = KeyState::PRESSED;
    }
    else if (action == GLFW_RELEASE) {
        keys[key] = KeyState::RELEASED;
    }
    else if (action == GLFW_REPEAT) {
        keys[key] = KeyState::HELD;
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        mouseButtons[button] = KeyState::PRESSED;
    }
    else if (action == GLFW_RELEASE) {
        mouseButtons[button] = KeyState::RELEASED;
    }
}

void Input::MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

void Input::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollX = xoffset;
    scrollY = yoffset;
}
