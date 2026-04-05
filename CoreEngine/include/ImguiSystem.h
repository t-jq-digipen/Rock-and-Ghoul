/* ========================================================
 * @file        ImguiSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 50%
 * @co-author   Tan Jun Quan       | t.junquan@digipen.edu 30%
 * @co-author   Manfred Sim        | manfred.sim@digipen.edu 20%
 * @date        06 NOV 2025
 *
 * @brief
 * Implements the ImGuiSystem, which provides the in-game editor
 * and debugging interface for the engine.
 *
 * - Engine Info: Displays framerate and system timings.
 * - Collision: Shows collider information, spatial grid details, collision debugging options and collision layer matrix.
 * - Scene: Handles scene loading, saving, backgrounds, and scene configuration
 * - Entity Editor + Prefabs: Allows editing, adding, duplicating, deleting, and saving entities and prefabs.
 * - Assets: Provides a browser for textures and audio files with drag-and-drop import support.
 * - Game View: Renders the current scene with play/stop simulation controls.
 * - Editor Camera: Provides a free-camera overview for navigating the level during editing.
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#ifndef NO_IMGUI
#pragma once
#include "System.h"
#include "Debug.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "DataDefs.h"

 /**
  * @class ImGuiSystem
  * @brief Provides the in-game editor and debugging interface.
  *
  * Integrates ImGui into the engine lifecycle, offering a fully dockable
  * workspace that consolidates multiple panels for real-time inspection,
  * editing, and debugging. The editor can be toggled at runtime (default: F1).
  *
  * Panels include:
  * - **Engine Info:** Displays framerate, frame time, and per-system timings.
  * - **Collision + Scene:** Visualizes collider stats, spatial grid data,
  *   and provides scene load/save management.
  * - **Entity Editor + Prefabs:** Lists all entities, supports component
  *   editing, prefab saving/loading, and runtime entity manipulation.
  * - **Assets:** Provides an asset browser with image thumbnails, drag-and-drop
  *   import for textures and audio, and FMOD audio preview.
  * - **Game View:** Renders the current scene with Play/Stop simulation controls.
  * - **Editor Camera:** Offers a minimap-style overview for panning and zooming.
  *
  * The system also supports entity dragging, prefab placement, and real-time
  * debug overlays for colliders, velocity vectors, and spatial grid visualization.
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
    static bool showSpatialGrid;
    static bool guiEnabled;
    static void OnFileDropped(const std::string& path);
    static bool dragEditMode;
    static void ShowLayerCollisionMatrix();

    static void UpdateSpawnCamera();

    static Vec2 camSpawnPos;
    static float camSpawnZoom;

    static Vec2 sceneMin;
    static Vec2 sceneMax;

    static Vec2 sceneSize;

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
    /**
     * @brief Displays the Asset Preview window in the editor.
     *
     * Provides a visual browser for assets such as textures and audio files.
     * Supports drag-and-drop importing, thumbnail previews, and audio playback.
     */
    void ShowAssetPreviewWindow();

    /**
     * @brief Renders the Editor Camera minimap.
     *
     * Draws a top-down view of the level showing all entities and the current
     * camera bounds. Allows panning and zooming to reposition the editor camera.
     *
     * viewportPos  The top-left corner of the camera panel.
     * viewportSize The available size of the camera viewport.
     */
    void ShowEntityCamera(ImVec2 viewportPos, ImVec2 viewportSize);

    /**
     * @brief Draws draggable handles for entities in the Game View.
     *
     * Displays small interactive handles for each entity’s position.
     * Entities can be selected and repositioned in real time using mouse drag.
     *
     * panelPos   The Game View’s screen-space position.
     * panelSize  The Game View’s size for coordinate scaling.
     */
    // void DrawEntityDragHandles(ImVec2 panelPos, ImVec2 panelSize);

    /**
    * @brief Displays prefab management controls.
    *
    * Enables saving selected entities as prefabs, deleting existing prefabs,
    * and activating prefab placement mode for spawning directly in the Game View.
    */
    void ShowPrefabControls();

    /**
     * @brief Displays the Scene Browser panel.
     *
     * Lists all available scene files, allowing users to create, load, save,
     * or delete scenes directly from the editor.
     */
    void ShowSceneBrowser();

    /**
     * @brief Renders play and stop controls for simulation.
     *
     * Allows switching between edit and play modes. Temporarily saves the
     * current scene before simulation and restores it when editing resumes.
     *
     * isPlaying flag indicating current state.
     */
    void ShowSimulationControls(bool& isPlaying);

    /**
     * @brief Displays component management.
     *
     * Allows adding or removing components while respecting dependency rules.
     *
     */
    void ShowAddRemoveComponents();

    /**
     * @brief Displays the Build Size Analyzer panel.
     *
     * Lists all assets in the project with file sizes, allows checkbox
     * selection, shows a running total, and exports selected assets to
     * a chosen destination folder.
     */
    void ShowBuildSizeAnalyzer();
};

#endif