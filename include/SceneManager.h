#pragma once
#include <string>

class SceneManager
{
public:
    static SceneManager& Get();
    // --- Simulation State ---
    bool isPlaying = true;    // True = gameplay
    bool isPaused = false;    // Optional pause
    bool inEditMode = false;     // True = editor mode

    // Optional helper functions
    inline bool IsPlaying() const { return isPlaying; }
    inline bool IsEditor()  const { return inEditMode; }
    inline bool IsPaused()  const { return isPaused; }
    void TogglePause() { isPaused = !isPaused; }
    void EnterPlayMode();
    void EnterEditMode();

    void LoadScene(const std::string& path);
    void ReloadScene();
    void SaveScene(const std::string& path);
    void Update(float dt, float fixedDt, int numSteps);
    void Draw();
    void QueueLoadScene(const std::string& path);
    const std::string& GetCurrentScene() const { return currentScenePath; }
    const std::string& GetTempScenePath() const { return tempScenePath; }
    std::string GetCurrentSceneName() const;

private:
    SceneManager() = default;

    std::string currentScenePath;      // Which .json is currently loaded
    std::string tempScenePath = "../Assets/saves/temp.json";         // Auto-generated temp version
    std::string pendingScene = "";
};
