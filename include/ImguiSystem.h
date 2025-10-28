/* ========================================================
 * @file        ImGuiSystem.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
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
#include "DataDefs.h"

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
    static void OnFileDropped(const std::string& path);
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

    void ShowEntityInteractionOverlay(ImVec2 viewportPos, ImVec2 viewportSize);
};

