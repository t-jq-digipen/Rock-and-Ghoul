#pragma once
#include "System.h"
#include "Debug.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class ImGuiSystem : public ISystem {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Render();
    void Shutdown() override;
    static Entity selectedEntity;
    static void InitializeImGui(GLFWwindow* window);
    static void ShutdownImGui();
    static bool showColliders;
    static bool showVelocity;
    static bool guiEnabled;

private:
    void ShowEngineDebugWindow();
    void ShowCollisionDebugWindow();
    void ShowHierarchyWindow();
    void ShowInspectorWindow();
};