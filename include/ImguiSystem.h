/* ========================================================
 * @file        ImGuiSystem.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date        26 OCT 2025
 *
 * @brief
 *
 * Declares the ImGuiSystem, an engine subsystem that manages the in-game editor/debug UI.
 * 
 * Features:
 * - Toggles on/off with a key press (F1).
 * - Renders a single "Game Editor" window with selectable panels:
 *   - Engine Info: frame timings and system performance
 *   - Collision: collider stats, entity positions, and scene save/load
 *   - Hierarchy + : entity list with a editor
 *
 * Integrates with the engine's system interface (ISystem) and
 * provides hooks for initialization, update, render, and shutdown.
 * 
 * Declares Editor data to extract editable values from components to
 * be utilized in the editor.
 * For each component, developer MUST provide an interface function to
 * acquire this data in order to use the editor.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "System.h"
#include "Debug.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <variant>
#include <map>

/**
 * @class ImGuiSystem
 * @brief Provides the in-game debug/editor interface.
 *
 * Implements the ISystem interface to integrate ImGui into the
 * engine lifecycle. The system exposes multiple panels:
 * - Engine Info: Displays frame rate and system timings.
 * - Collision: Shows collider stats, entity positions, and scene save/load.
 * - Hierarchy + Editor: Lists all entities and allows inspecting/editing
 *   their components side by side.
 *
 * The editor can be toggled at runtime (default: F1).
 */
class ImGuiSystem : public ISystem {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Render();
    void Shutdown() override;
    static Entity selectedEntity;
    static void InitializeImGui();
    static void ShutdownImGui();
    static bool showColliders;
    static bool showVelocity;
    static bool guiEnabled;

private:
    /**
     * @brief Draw the Engine Debug panel.
     *
     * Displays framerate, frame time, and per-system timings.
     */
    void ShowEngineDebugWindow();
    /**
     * @brief Draw the Collision Debug panel.
     *
     * Shows active colliders, entity positions, and scene management
     * controls (save/load). Includes toggles for collider and velocity
     * visualization.
     */
    void ShowCollisionDebugWindow();
    /**
     * @brief Draw the Hierarchy panel.
     *
     * Lists all entities with their tags and allows selection or
     * creation of new entities.
     */
    void ShowHierarchyWindow();
    /**
     * @brief Draw the Inspector panel.
     *
     * Shows and allows editing of components for the currently
     * selected entity (transform, shape, sprite, physics, etc.).
     */
    void ShowInspectorWindow();

    void ShowAssetPreviewWindow();
};

namespace Editor {
    // Determines the type of editing widget
    enum WidgetType {
        WIDGET_CHECKMARK,
        WIDGET_DROPDOWN,
        WIDGET_INT,
        WIDGET_FLOAT,
        WIDGET_STRING
    };

    // Widget data variants based on widget type
#pragma region WidgetData
    struct Checkmark {
        bool check;
        bool* ref = nullptr;
    };

    struct Text {
        std::string string;
        std::string* ref = nullptr;
    };

    struct Dropdown {
        std::vector<std::string> options;
        std::unordered_map<std::string, int> enumMap;
        int* ref = nullptr;

    };

    struct Widget_Int {
        bool slider = false;
        int min;
        int max;
        int* ref = nullptr;
    };

    struct Widget_Float {
        bool slider = false;
        float min;
        float max;
        float* ref = nullptr;
    };

    using WidgetData = std::variant<Checkmark, Text, Dropdown, Widget_Int, Widget_Float>;

#pragma endregion WidgetData



    struct DataField {
        WidgetType type;
        WidgetData data;
    };

    // Keyed with VALUE NAME
    using DataName = std::string;

    // Keyed with VALUE NAME
    // Data holds 
    class ComponentData {
    private:
        std::string ComponentName;
        std::map<DataName, DataField> ComponentInfo;
    public:
        ComponentData(const std::string& name) : ComponentName{ name } {}

        void Add_Data(const DataName& name, const DataField& dataf) {
            ComponentInfo[name] = dataf;
        }

    };

};