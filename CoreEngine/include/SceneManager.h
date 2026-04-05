/* ========================================================
 * @file        SceneManager.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 60%
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu 40%
 * @date        29 NOV 2025
 *
 * @brief
 * Implements the SceneManager, which controls scene loading,
 * unloading, transitioning, play/edit modes, and scene state.
 *
 * - Handles safe scene loading using deferred loading (queued load).
 * - Manages scene transitions with optional fade timing.
 * - Controls PLAY mode, PAUSE state, and EDIT mode switching.
 * - Integrates with JsonLoad for scene serialization/deserialization.
 * - Updates camera mode depending on play/edit state.
 * - Ensures audio is stopped when entering edit mode.
 * - Provides scene saving and reloading functionality.
 * - Stores current scene metadata (name, path, temporary paths).
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include "DataDefs.h"
#include "FilePath.h"


/**
 * @class SceneManager
 * @brief Singleton that manages scene lifecycle, simulation state, and transitions.
 *
 * Responsible for loading, saving, reloading, and transitioning between scenes.
 * Controls whether the engine is in play mode, edit mode, or paused.
 * Scene loads are deferred to safe frame boundaries via QueueLoadScene().
 */
class SceneManager
{
public:
    /**
     * @brief Returns the singleton instance of SceneManager.
     * @return Reference to the single SceneManager instance.
     */
    static SceneManager& Get();

    // --- Simulation State ---
#ifdef NO_IMGUI
    bool isPlaying  = true;    // game is actively running (Release default)
    bool isPaused   = false;   // gameplay is paused
    bool inEditMode = false;   //  editor is active (always false in Release)
#else
    bool isPlaying  = false;   // the game is actively running (Editor default is stop)
    bool isPaused   = true;    //gameplay is paused (Editor default: paused)
    bool inEditMode = true;    // editor is active (Editor default: editing)
#endif

    /**
     * @brief Returns whether the game is currently in play mode.
     * @return True if isPlaying is set.
     */
    inline bool IsPlaying() const { return isPlaying; }

    /**
     * @brief Returns whether the editor is currently active.
     * @return True if inEditMode is set.
     */
    inline bool IsEditor()  const { return inEditMode; }

    /**
     * @brief Returns whether gameplay is currently paused.
     * @return True if isPaused is set.
     */
    inline bool IsPaused()  const { return isPaused; }

    /**
     * @brief Sets the paused state directly.
     * @param p True to pause, false to unpause.
     */
    void SetPaused(bool p) { isPaused = p; }

    /**
     * @brief Toggles the paused state between true and false.
     */
    void TogglePause() { isPaused = !isPaused; }

    /**
     * @brief Switches the engine into play mode.
     *        Saves the current scene to a temp file, disables edit mode,
     *        sets isPlaying to true, and restores scene camera/audio settings.
     */
    void EnterPlayMode();

    /**
     * @brief Switches the engine back into edit mode.
     *        Reloads the temp scene snapshot, stops audio, enables the editor,
     *        and resets the camera to editor defaults.
     */
    void EnterEditMode();

    /**
     * @brief Immediately loads and deserializes a scene from a JSON file.
     *        Clears all current entities and replaces them with those in the file.
     * @param path Absolute or asset-relative path to the scene JSON file.
     */
    void LoadScene(const std::string& path);

    /**
     * @brief Reloads the currently loaded scene from disk, discarding all runtime changes.
     */
    void ReloadScene();

    /**
     * @brief Serializes and saves the current scene state to a JSON file.
     * @param path           Destination path for the saved scene JSON.
     * @param camSettings    Optional camera settings to embed in the save file.
     */
    void SaveScene(const std::string& path, const SavedScene::SavedCamera& camSettings = {});

    /**
     * @brief Runs per-frame scene logic: processes deferred scene loads,
     *        advances the fade transition timer, and handles hotkeys (P to pause).
     * @param dt        Delta time in seconds since last frame.
     * @param fixedDt   Fixed physics timestep in seconds.
     * @param numSteps  Number of fixed-timestep steps to run this frame.
     */
    void Update(float dt, float fixedDt, int numSteps);

    /**
     * @brief Submits any scene-level draw calls (e.g. transition overlays).
     */
    void Draw();

    /**
     * @brief Queues a scene to be loaded at the start of the next safe frame boundary.
     *        Safe to call from within gameplay logic or collision callbacks.
     * @param path Path to the scene JSON file to load.
     */
    void QueueLoadScene(const std::string& path);

    /**
     * @brief Returns the file path of the currently loaded scene.
     * @return Const reference to the current scene path string.
     */
    const std::string& GetCurrentScene() const { return currentScenePath; }

    /**
     * @brief Returns the path to the temporary scene snapshot used by play/edit mode switching.
     * @return Const reference to the temp scene path string.
     */
    const std::string& GetTempScenePath() const { return tempScenePath; }

    /**
     * @brief Extracts and returns just the filename (without directory or extension) of the current scene.
     * @return Scene name string derived from the current scene path.
     */
    std::string GetCurrentSceneName() const;

    /**
     * @brief Applies default rendering and camera settings for the current scene type.
     *        Called after a scene is loaded to restore scene-specific configuration.
     */
    void ApplySceneDefaults();

    /**
     * @brief Returns the type of the currently loaded scene (e.g. ST_MENU, ST_LEVEL, ST_CUTSCENE).
     * @return The current SceneType enum value.
     */
    SceneType GetSceneType() { return sceneType; }

    /**
     * @brief Sets the type of the currently loaded scene.
     * @param type The SceneType to assign to the current scene.
     */
    void SetSceneType(SceneType type) { sceneType = type; }

    /**
     * @brief Returns whether the engine is currently in edit mode.
     *        Alias for IsEditor(); used to gate audio and gameplay systems.
     * @return True if inEditMode is set.
     */
    bool IsInEditMode() const { return inEditMode; }

    /**
     * @brief Stores the asset name of the BGM track for the current scene.
     * @param bgmAssetName Asset name string (key into the audio manifest).
     */
    void SetCurrentSceneBGM(const std::string& bgmAssetName) { currentSceneBGM = bgmAssetName; }

    /**
     * @brief Returns the asset name of the BGM track for the current scene.
     * @return The BGM asset name string.
     */
    std::string GetCurrentSceneBGM() const { return currentSceneBGM; }

    /**
     * @brief Stores the asset name of the optional second BGM layer for the current scene.
     * @param name Asset name string for the secondary BGM layer.
     */
    void SetCurrentSceneBGMLayer(const std::string& name) { currentSceneBGMLayer = name; }

    /**
     * @brief Returns the asset name of the optional second BGM layer for the current scene.
     * @return The secondary BGM layer asset name string.
     */
    std::string GetCurrentSceneBGMLayer() const { return currentSceneBGMLayer; }

    /**
     * @brief Sets the duration of the scene fade transition in seconds.
     * @param duration Fade duration in seconds. Use 0 for an instant transition.
     */
    void SetTransitionTime(float duration) { sceneFadeDuration = duration; }

    /**
     * @brief Returns the normalized progress of the current scene fade transition [0, 1].
     *        Returns 1.0 immediately if no fade duration is set.
     * @return Float in [0, 1] where 1 means fully faded in (transition complete).
     */
    float GetTransitionProgress() const { return (sceneFadeDuration == 0.f) ? 1.f : sceneTransitionTime / sceneFadeDuration; }

    /**
     * @brief Returns whether a cutscene is currently playing.
     * @return True if a cutscene is active.
     */
    bool IsCutscenePlaying() const { return m_cutscenePlaying; }

    /**
     * @brief Sets whether a cutscene is currently playing.
     * @param v True to mark a cutscene as active, false to clear it.
     */
    void SetCutscenePlaying(bool v) { m_cutscenePlaying = v; }


private:
    /** @brief Private constructor — use Get() to access the singleton. */
    SceneManager() = default;

    std::string currentScenePath;      ///< Path to the currently loaded scene JSON.
    std::string tempScenePath = (FilePath::ASSETS_FOLDER + "saves/temp.json"); ///< Temp snapshot path used during play/edit switching.
    std::string currentSceneBGM;       ///< Asset name of the current scene's BGM track.
    std::string currentSceneBGMLayer;  ///< Asset name of the optional secondary BGM layer.
    std::string pendingScene = "";     ///< Scene path queued for deferred loading; empty if none pending.
    float sceneTransitionTime = 0.f;   ///< Current elapsed time of the fade transition (counts up to sceneFadeDuration).
    float sceneFadeDuration   = 0.f;   ///< Total duration of the fade transition in seconds; 0 = instant.
    SceneType sceneType = ST_MENU;     ///< Type of the currently loaded scene.
    bool m_cutscenePlaying = false;    ///< True when a cutscene is currently active.
};
