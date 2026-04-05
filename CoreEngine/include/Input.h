/* ========================================================
 * @file        Input.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu 90%
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu 10%
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

/**
 * @struct KeyPressedEvent
 * @brief Event published to the EventBus when a key transitions to PRESSED state.
 */
struct KeyPressedEvent : public IEvent {
    int key; // GLFW key token that was pressed.
};

/**
 * @struct KeyHeldEvent
 * @brief Event published to the EventBus each frame a key remains in HELD state.
 */
struct KeyHeldEvent : public IEvent {
    int key; // GLFW key token that is being held.
};

/**
 * @struct KeyReleasedEvent
 * @brief Event published to the EventBus when a key transitions to RELEASED state.
 */
struct KeyReleasedEvent : public IEvent {
    int key; // GLFW key token that was released.
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
    /**
     * @brief Returns true if the key was pressed this frame.
     * @param key GLFW key token (e.g. GLFW_KEY_SPACE).
     * @return True if the key state is PRESSED.
     */
    static bool IsKeyPressed(int key);

    /**
     * @brief Returns true if the key is being held down.
     * @param key GLFW key token.
     * @return True if the key state is HELD.
     */
    static bool IsKeyHeld(int key);

    /**
     * @brief Returns true if the key was released this frame.
     * @param key GLFW key token.
     * @return True if the key state is RELEASED.
     */
    static bool IsKeyReleased(int key);

    /**
     * @brief Returns true if the mouse button was pressed this frame.
     * @param button GLFW mouse button token (e.g. GLFW_MOUSE_BUTTON_LEFT).
     * @return True if the button state is PRESSED.
     */
    static bool IsMousePressed(int button);

    /**
     * @brief Returns true if the mouse button is being held down.
     * @param button GLFW mouse button token.
     * @return True if the button state is HELD.
     */
    static bool IsMouseHeld(int button);

    /**
     * @brief Returns true if the mouse button was released this frame.
     * @param button GLFW mouse button token.
     * @return True if the button state is RELEASED.
     */
    static bool IsMouseReleased(int button);

    /**
     * @brief Returns the current cursor X position in window coordinates.
     * @return Cursor X as a double.
     */
    static double GetMouseX();

    /**
     * @brief Returns the current cursor Y position in window coordinates.
     * @return Cursor Y as a double.
     */
    static double GetMouseY();

    /**
     * @brief Returns the horizontal scroll offset since the last frame.
     * @return Horizontal scroll delta as a double.
     */
    static double GetScrollX();

    /**
     * @brief Returns the vertical scroll offset since the last frame.
     * @return Vertical scroll delta as a double.
     */
    static double GetScrollY();

    /**
     * @brief Returns the current cursor position as a Vec2.
     * @return Vec2 containing (mouseX, mouseY) in window coordinates.
     */
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

    // Gamepad query functions
    // axis = GLFW_GAMEPAD_AXIS_LEFT_X/Y, RIGHT_X/Y, LEFT_TRIGGER, RIGHT_TRIGGER
    /**
     * @brief Returns the current value of a gamepad axis with deadzone applied.
     * @param axis GLFW gamepad axis token (e.g. GLFW_GAMEPAD_AXIS_LEFT_X).
     * @return Axis value in [-1, 1], or 0 if no gamepad is connected.
     */
    static float GetGamepadAxis(int axis);

    /**
     * @brief Returns whether a gamepad is currently connected.
     * @return True if an active joystick/gamepad is detected.
     */
    static bool  IsGamepadConnected();

    // btn = GLFW_GAMEPAD_BUTTON_* (raw per-button state, independent of keyboard emulation)
    /**
     * @brief Returns true if the gamepad button was pressed this frame.
     * @param btn GLFW gamepad button token (e.g. GLFW_GAMEPAD_BUTTON_A).
     * @return True if the button state is PRESSED.
     */
    static bool IsGamepadButtonPressed(int btn);

    /**
     * @brief Returns true if the gamepad button is being held down.
     * @param btn GLFW gamepad button token.
     * @return True if the button state is HELD.
     */
    static bool IsGamepadButtonHeld(int btn);

    /**
     * @brief Returns true if the gamepad button was released this frame.
     * @param btn GLFW gamepad button token.
     * @return True if the button state is RELEASED.
     */
    static bool IsGamepadButtonReleased(int btn);

    /**
     * @brief Polls the active gamepad and updates axis values and button states.
     *        Call once per frame after glfwPollEvents().
     */
    static void PollGamepad();

    /**
     * @brief GLFW joystick connection/disconnection callback.
     *        Updates the active joystick ID when a gamepad is plugged in or removed.
     * @param jid   Joystick ID that triggered the event.
     * @param event GLFW_CONNECTED or GLFW_DISCONNECTED.
     */
    static void JoystickCallback(int jid, int event);

private:

    // Active joystick ID for gamepad (-1 if none connected)
    static int   activeJoystickId;
    // Current gamepad axis values [GLFW_GAMEPAD_AXIS_LAST + 1], deadzone applied
    static float gamepadAxes[6];
    // Button states from previous frame (GLFW_PRESS / GLFW_RELEASE)
    static unsigned char prevGamepadButtons[15];
    // Per-button KeyState for direct queries (GLFW_GAMEPAD_BUTTON_*)
    static KeyState gamepadButtonStates[15];

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
