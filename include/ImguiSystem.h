#pragma once
#include "System.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class ImGuiSystem : public ISystem {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Render();
    void Shutdown() override;

    static void InitializeImGui(GLFWwindow* window);
    static void ShutdownImGui();

private:
    void ShowEngineDebugWindow();
    void ShowCollisionDebugWindow();
    void ShowEntityDebugWindow();
};