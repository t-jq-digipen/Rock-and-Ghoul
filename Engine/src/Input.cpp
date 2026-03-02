/* ========================================================
 * @file        Input.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date        04 NOV 2025
 *
 * @brief
 * 
 * Implements the Input system, which handles keyboard and mouse input via GLFW callbacks. 
 * This module maintains the current state of keys and mouse buttons (Pressed,
 * Held, Released), tracks mouse position, and processes scroll offsets.
 * Transient states are updated each frame to ensure consistent input.
 * 
 * EDIT: 04 NOV
 * Key inputs now generate events which are pushed to the EventBus.
 * These event pushes are sent in Input::KeyCallback.
 * 
 * 
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#include "glapp.h"
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
void Input::KeyCallback(GLFWwindow* , int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (action == GLFW_PRESS) {
        KeyPressedEvent keypress;
        keypress.key = key;
        EventBus::Get().PushEvent(keypress);
        keys[key] = KeyState::PRESSED;
    }
    else if (action == GLFW_RELEASE) {
        KeyReleasedEvent keyrelease;
        keyrelease.key = key;
        EventBus::Get().PushEvent(keyrelease);
        keys[key] = KeyState::RELEASED;
    }
    else if (action == GLFW_REPEAT) {
        KeyHeldEvent keyheld;
        keyheld.key = key;
        EventBus::Get().PushEvent(keyheld);
        keys[key] = KeyState::HELD;
    }
    //if (key == GLFW_KEY_ESCAPE) {
    //    glfwSetWindowShouldClose(window, GLFW_TRUE);
    //}
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && (mods & GLFW_MOD_ALT)) {
        GLApp::ToggleFullscreen();
        return;
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    if (action == GLFW_PRESS) {
        mouseButtons[button] = KeyState::PRESSED;
    }
    else if (action == GLFW_RELEASE) {
        mouseButtons[button] = KeyState::RELEASED;
    }
}

void Input::MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    mouseX = xpos;
    mouseY = ypos;
}

void Input::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    scrollX = xoffset;
    scrollY = yoffset;
}

Vec2 Input::GetMousePosition() {
    return Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}