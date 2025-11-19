#include "SceneManager.h"
#include "JsonLoad.h"
#include "RenderSystem.h"
#include "GuiSystem.h"
#include "Render.h"
#include "Registry.h"
#include "Camera.h"
#include "LogicSystem.h"
#include <filesystem>

SceneManager& SceneManager::Get()
{
    static SceneManager instance;
    return instance;
}

void SceneManager::LoadScene(const std::string& path)
{
    // Queue the scene load (do NOT load now)
    pendingScene = path;
}
void SceneManager::QueueLoadScene(const std::string& path)
{
    pendingScene = path;
}
void SceneManager::Update(float dt, float fixedDt, int numSteps)
{
    // Only load scenes at SAFE time (start of frame)
    if (!pendingScene.empty())
    {
        std::string path = pendingScene;
        pendingScene.clear();

        // *** SAFE LOAD HERE ***
        JsonLoad::ClearAllEntities();

        if (JsonLoad::LoadScene(path))
        {
            currentScenePath = path;

            auto p = std::filesystem::path(path);
            tempScenePath = (p.parent_path() / (p.stem().string() + "_temp" + p.extension().string())).string();

            std::cout << "[SceneManager] Loaded scene: " << path << "\n";
        }
        else
        {
            std::cerr << "[SceneManager] FAILED to load scene: " << path << "\n";
        }
    }
}


void SceneManager::SaveScene(const std::string& path)
{
    std::string sceneName = std::filesystem::path(path).stem().string();
    JsonLoad::SaveCurrentScene(path, sceneName);
    std::cout << "[SceneManager] Saved scene: " << path << "\n";
}

void SceneManager::ReloadScene()
{
    if (!currentScenePath.empty())
        LoadScene(currentScenePath);
}


void SceneManager::Draw()
{
    // kept for future if needed
}

void SceneManager::EnterPlayMode()
{
    isPlaying = true;
    isPaused = false;
    inEditMode = false;

    Camera& cam = Camera::GetInstance();
    cam.SetMode(Camera::Mode::GAME);
    cam.SetZoom(1.0f);
    cam.ResetToCenter();

    LogicSystem::Get().Initialize();

    std::cout << "[SceneManager] Entered PLAY MODE.\n";
}

void SceneManager::EnterEditMode()
{
    isPlaying = false;
    isPaused = true;
    inEditMode = true;

    Camera& cam = Camera::GetInstance();
    cam.SetMode(Camera::Mode::EDITOR);
    cam.ResetToCenter();

    std::cout << "[SceneManager] Entered EDIT MODE.\n";
}

std::string SceneManager::GetCurrentSceneName() const
{
    if (currentScenePath.empty())
        return "";

    return std::filesystem::path(currentScenePath).stem().string();
}