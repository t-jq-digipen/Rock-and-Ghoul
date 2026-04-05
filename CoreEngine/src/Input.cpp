/* ========================================================
 * @file        Input.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 90%
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu 10%
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
#include <cstring>

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

int          Input::activeJoystickId = -1;
float        Input::gamepadAxes[6]   = {};
unsigned char Input::prevGamepadButtons[15] = {};
KeyState     Input::gamepadButtonStates[15] = {};

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

// -----------------------------
// Gamepad Functions
// -----------------------------
void Input::PollGamepad() {
    if (activeJoystickId == -1) return;

    GLFWgamepadstate state;
    if (!glfwGetGamepadState(activeJoystickId, &state)) {
        activeJoystickId = -1;
        std::memset(prevGamepadButtons, GLFW_RELEASE, sizeof(prevGamepadButtons));
        std::memset(gamepadAxes, 0, sizeof(gamepadAxes));
        return;
    }

    // Update axis values with deadzone applied
    constexpr float DEADZONE = 0.15f;
    for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; ++i)
        gamepadAxes[i] = (state.axes[i] > DEADZONE || state.axes[i] < -DEADZONE) ? state.axes[i] : 0.f;

    // Update raw per-button states for direct queries (e.g. D-pad in menus)
    for (int b = 0; b <= GLFW_GAMEPAD_BUTTON_LAST; ++b) {
        const bool isDown  = (state.buttons[b] == GLFW_PRESS);
        const bool wasDown = (prevGamepadButtons[b] == GLFW_PRESS);
        if      ( isDown && !wasDown) gamepadButtonStates[b] = KeyState::PRESSED;
        else if ( isDown &&  wasDown) gamepadButtonStates[b] = KeyState::HELD;
        else if (!isDown &&  wasDown) gamepadButtonStates[b] = KeyState::RELEASED;
        else                          gamepadButtonStates[b] = KeyState::NONE;
    }

    // Helper: map a gamepad button to an emulated keyboard key (press/held/release)
    auto emitKey = [&](int gpBtn, int kbKey) {
        const bool isDown  = (state.buttons[gpBtn] == GLFW_PRESS);
        const bool wasDown = (prevGamepadButtons[gpBtn] == GLFW_PRESS);
        if (isDown && !wasDown) {
            keys[kbKey] = KeyState::PRESSED;
            KeyPressedEvent e; e.key = kbKey;
            EventBus::Get().PushEvent(e);
        } else if (!isDown && wasDown) {
            if (keys[kbKey] == KeyState::HELD || keys[kbKey] == KeyState::PRESSED) {
                keys[kbKey] = KeyState::RELEASED;
                KeyReleasedEvent e; e.key = kbKey;
                EventBus::Get().PushEvent(e);
            }
        } else if (isDown && wasDown && keys[kbKey] == KeyState::NONE) {
            keys[kbKey] = KeyState::HELD;
        }
    };

    // Left Trigger held = song/spell combo input mode
    const bool ltMod = (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.3f);

    if (ltMod) {
        // LT + face buttons = directional spell combo (arrow keys)
        // Y=Up, X=Left, B=Right, A=Down  (matches physical button positions)
        emitKey(GLFW_GAMEPAD_BUTTON_Y, GLFW_KEY_UP);
        emitKey(GLFW_GAMEPAD_BUTTON_X, GLFW_KEY_LEFT);
        emitKey(GLFW_GAMEPAD_BUTTON_B, GLFW_KEY_RIGHT);
        emitKey(GLFW_GAMEPAD_BUTTON_A, GLFW_KEY_DOWN);
    } else {
        // Normal face button functions
        emitKey(GLFW_GAMEPAD_BUTTON_X, GLFW_KEY_1);          // Spell slot 1
        emitKey(GLFW_GAMEPAD_BUTTON_Y, GLFW_KEY_2);          // Spell slot 2
        emitKey(GLFW_GAMEPAD_BUTTON_B, GLFW_KEY_3);          // Spell slot 3
        emitKey(GLFW_GAMEPAD_BUTTON_A, GLFW_KEY_LEFT_SHIFT); // Dash
        // A also acts as confirm/advance in menus and stage transitions
        emitKey(GLFW_GAMEPAD_BUTTON_A, GLFW_KEY_ENTER);
        emitKey(GLFW_GAMEPAD_BUTTON_A, GLFW_KEY_SPACE);
    }

    // Fixed button mappings (not affected by LT modifier)
    emitKey(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_KEY_E);      // Melee attack
    emitKey(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,  GLFW_KEY_Q);      // Cycle element
    emitKey(GLFW_GAMEPAD_BUTTON_START,        GLFW_KEY_ESCAPE); // Pause
    emitKey(GLFW_GAMEPAD_BUTTON_BACK,         GLFW_KEY_ESCAPE); // Back

    // D-pad: raw state only (queried directly via IsGamepadButtonPressed — no key events fired,
    //         so D-pad never accidentally triggers spell combos or character movement)

    // Save button states for next-frame transition detection
    for (int b = 0; b <= GLFW_GAMEPAD_BUTTON_LAST; ++b)
        prevGamepadButtons[b] = state.buttons[b];
}

void Input::JoystickCallback(int jid, int event) {
    if (event == GLFW_CONNECTED) {
        if (activeJoystickId == -1 && glfwJoystickIsGamepad(jid)) {
            activeJoystickId = jid;
            std::cout << "[Input] Gamepad connected: " << glfwGetGamepadName(jid) << "\n";
        }
    } else if (event == GLFW_DISCONNECTED) {
        if (jid == activeJoystickId) {
            activeJoystickId = -1;
            std::memset(prevGamepadButtons, GLFW_RELEASE, sizeof(prevGamepadButtons));
            std::memset(gamepadAxes, 0, sizeof(gamepadAxes));
            std::cout << "[Input] Gamepad disconnected.\n";
            // Find another connected gamepad if available
            for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
                if (i != jid && glfwJoystickPresent(i) && glfwJoystickIsGamepad(i)) {
                    activeJoystickId = i;
                    std::cout << "[Input] Switched to gamepad: " << glfwGetGamepadName(i) << "\n";
                    break;
                }
            }
        }
    }
}

float Input::GetGamepadAxis(int axis) {
    if (axis < 0 || axis > GLFW_GAMEPAD_AXIS_LAST) return 0.f;
    return gamepadAxes[axis];
}

bool Input::IsGamepadConnected() {
    return activeJoystickId != -1;
}

bool Input::IsGamepadButtonPressed(int btn) {
    if (btn < 0 || btn > GLFW_GAMEPAD_BUTTON_LAST) return false;
    return gamepadButtonStates[btn] == KeyState::PRESSED;
}

bool Input::IsGamepadButtonHeld(int btn) {
    if (btn < 0 || btn > GLFW_GAMEPAD_BUTTON_LAST) return false;
    return gamepadButtonStates[btn] == KeyState::HELD;
}

bool Input::IsGamepadButtonReleased(int btn) {
    if (btn < 0 || btn > GLFW_GAMEPAD_BUTTON_LAST) return false;
    return gamepadButtonStates[btn] == KeyState::RELEASED;
}