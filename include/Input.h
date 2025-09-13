#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <unordered_map>

enum class KeyState {
    RELEASED,
    PRESSED,
    HELD
};

class Input {
public:
    // Called once per frame to reset transient states (like PRESSED/RELEASED)
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

    // GLFW callbacks (set in GLApp::init)
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MousePosCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    static std::unordered_map<int, KeyState> keys;
    static std::unordered_map<int, KeyState> mouseButtons;
    static double mouseX, mouseY;
    static double scrollX, scrollY;
};

#endif // INPUT_H
