/* ========================================================
 * @file        Input.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date        04 NOV 2025
 *
 * @brief
 *
 * Declares the Input class, which manages keyboard and mouse input for the engine.
 * Input is captured via GLFW callbacks and stored in static maps of key/button states. 
 * The system tracks transitions (Pressed, Held, Released, None), cursor position, and scroll offsets. 
 * Query functions provide easy access to the current input state, while Update() ensures
 * states last only one frame.
 * 
 * EDIT: 04 NOV
 * Key inputs now push events to EventBus.
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <unordered_map>
#include "Remath.h"
#include "EventBus.h"
/**
 * @enum KeyState
 * @brief Represents the possible states of a key or mouse button.
 *
 * - PRESSED : Key/button was pressed this frame (transient, lasts one frame).
 * - HELD    : Key/button is being held down across frames.
 * - RELEASED: Key/button was released this frame (transient, lasts one frame).
 * - NONE    : Neutral state (no input).
 */
enum class KeyState {
    RELEASED,
    PRESSED,
    HELD,
    NONE
};

struct KeyPressedEvent : public IEvent {
    int key;
};

struct KeyHeldEvent : public IEvent {
    int key;
};

struct KeyReleasedEvent : public IEvent {
    int key;
};


/**
 * @class Input
 * @brief Static input manager that tracks keyboard and mouse states via GLFW callbacks.
 *
 * Provides per-frame update of states (Pressed/Released to Held/None),
 * and query functions to check the current state of keys and mouse buttons,
 * as well as mouse position and scroll offsets.
 */
class Input {
public:
    /**
     * @brief Called once per frame to update transient input states.
     *
     * Converts PRESSED to HELD and RELEASED to NONE to ensure one-frame accuracy
     * for key/button transitions.
     */
    static void Update();

    // Query functions
    static bool IsKeyPressed(int key);
    static bool IsKeyHeld(int key);
    static bool IsKeyReleased(int key);

    static bool IsMousePressed(int button);
    static bool IsMouseHeld(int button);
    static bool IsMouseReleased(int button);

    static double GetMouseX();
    static double GetMouseY();
    static double GetScrollX();
    static double GetScrollY();
    static Vec2 GetMousePosition();
    // GLFW callbacks (set in GLApp::init)
    /**
     * @brief GLFW key input callback.
     * @param window Pointer to the GLFW window.
     * @param key Keyboard key (GLFW_KEY_*).
     * @param scancode Hardware scancode (unused here).
     * @param action GLFW_PRESS, GLFW_RELEASE, or GLFW_REPEAT.
     * @param mods Modifier keys (Shift, Ctrl, etc).
     */
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
         * @brief GLFW mouse button callback.
         * @param window Pointer to the GLFW window.
         * @param button Mouse button (GLFW_MOUSE_BUTTON_*).
         * @param action GLFW_PRESS or GLFW_RELEASE.
         * @param mods Modifier keys (unused here).
         */
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    /**
    * @brief GLFW mouse position callback.
    * @param window Pointer to the GLFW window.
    * @param xpos Current X position in window coordinates.
    * @param ypos Current Y position in window coordinates.
    */
    static void MousePosCallback(GLFWwindow* window, double xpos, double ypos);
    /**
     * @brief GLFW mouse scroll callback.
     * @param window Pointer to the GLFW window.
     * @param xoffset Horizontal scroll offset.
     * @param yoffset Vertical scroll offset.
     */
    static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
private:

    // Map of key codes (GLFW_KEY_*) to their current KeyState.
    static std::unordered_map<int, KeyState> keys;
    // Map of mouse button codes (GLFW_MOUSE_BUTTON_*) to their current KeyState.
    static std::unordered_map<int, KeyState> mouseButtons;
    // Current mouse cursor X position.
    static double mouseX;
    // Current mouse cursor Y position.
    static double mouseY;
    // Horizontal scroll offset since last frame.
    static double scrollX;
    // Vertical scroll offset since last frame.
    static double scrollY;
    
};

#endif // INPUT_H
