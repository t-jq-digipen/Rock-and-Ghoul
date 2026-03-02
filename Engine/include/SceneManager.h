/* ========================================================
 * @file        SceneManager.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
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
 * Additional Features:
 * - Supports hotkeys for pause (P) and scene switching (H).
 * - Ensures scene loading happens only at safe points in the frame.
 * - Restores default scene camera and renderer settings.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include "DataDefs.h"


class SceneManager
{
public:
    static SceneManager& Get();
    // --- Simulation State ---
#ifdef NO_IMGUI
    bool isPlaying  = true;    // True = gameplay
    bool isPaused   = false;    // Optional pause
    bool inEditMode = false;     // True = editor mode
#else
    bool isPlaying  = false;    // True = gameplay
    bool isPaused   = true;    // Optional pause
    bool inEditMode = true;     // True = editor mode
#endif

    //helper functions
    inline bool IsPlaying() const { return isPlaying; }
    inline bool IsEditor()  const { return inEditMode; }
    inline bool IsPaused()  const { return isPaused; }
    void SetPaused(bool p) {
        isPaused = p;
    }

    void TogglePause() { isPaused = !isPaused; }
    void EnterPlayMode();
    void EnterEditMode();

    void LoadScene(const std::string& path);
    void ReloadScene();
    void SaveScene(const std::string& path, const SavedScene::SavedCamera& camSettings = {});
    void Update(float dt, float fixedDt, int numSteps);
    void Draw();
    void QueueLoadScene(const std::string& path);
    const std::string& GetCurrentScene() const { return currentScenePath; }
    const std::string& GetTempScenePath() const { return tempScenePath; }
    std::string GetCurrentSceneName() const;
    void ApplySceneDefaults();
    SceneType GetSceneType() { return sceneType; }
    void SetSceneType(SceneType type) { sceneType = type; }

    bool IsInEditMode() const { return inEditMode; } // to stop music in edit mode
    void SetCurrentSceneBGM(const std::string& bgmAssetName) { currentSceneBGM = bgmAssetName; }
    std::string GetCurrentSceneBGM() const { return currentSceneBGM; }

    void SetTransitionTime(float duration) { sceneFadeDuration = duration; }
    float GetTransitionProgress() const { return (sceneFadeDuration == 0.f) ? 1.f : sceneTransitionTime / sceneFadeDuration; }

    //Global flag for the cutscenes
    bool IsCutscenePlaying() const { return m_cutscenePlaying; }
    void SetCutscenePlaying(bool v) { m_cutscenePlaying = v; }


private:
    SceneManager() = default;

    std::string currentScenePath;      // Which .json is currently loaded
    std::string tempScenePath = "../Assets/saves/temp.json";
    std::string currentSceneBGM;  // BGM asset name for current scene
    std::string pendingScene = "";
    float sceneTransitionTime = 0.f; // goes from 1.f to 0.f
    float sceneFadeDuration = 0.f;
    SceneType sceneType = ST_MENU;
    bool m_cutscenePlaying = false;
};
