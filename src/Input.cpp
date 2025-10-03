/* ========================================================
 * @file        Input.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 * 
 * Implements the Input system, which handles keyboard and mouse input via GLFW callbacks. 
 * This module maintains the current state of keys and mouse buttons (Pressed,
 * Held, Released), tracks mouse position, and processes scroll offsets.
 * Transient states are updated each frame to ensure consistent input.
 * 
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "input.h"
#include <iostream>

//check if a key/button is pressed/held/released
//keys = all keyboard states.
//mouseButtons = all mouse button states.
//Each entry maps a GLFW key / button code(int) to a KeyState enum (PRESSED, HELD, RELEASED).
std::unordered_map<int, KeyState> Input::keys;
std::unordered_map<int, KeyState> Input::mouseButtons;
double Input::mouseX = 0.0;
double Input::mouseY = 0.0;
double Input::scrollX = 0.0;
double Input::scrollY = 0.0;

void Input::Update() {
    for (auto& [key, state] : keys) {
        if (state == KeyState::PRESSED) {
            state = KeyState::HELD;  // pressed to held (only first frame is "pressed")
        }
        else if (state == KeyState::RELEASED) {
            state = KeyState::NONE; // stays released for one frame
        }
    }

    for (auto& [button, state] : mouseButtons) {
        if (state == KeyState::PRESSED) {
            state = KeyState::HELD;
        }
        else if (state == KeyState::RELEASED) {
            state = KeyState::NONE;
        }
    }

    // Reset scroll each frame so it only reflects last tick scroll
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
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
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
