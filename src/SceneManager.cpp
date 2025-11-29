#include "SceneManager.h"
#include "JsonLoad.h"
#include "RenderSystem.h"
#include "GuiSystem.h"
#include "Render.h"
#include "Registry.h"
#include "Camera.h"
#include "LogicSystem.h"
#include <filesystem>
#include "ImguiSystem.h"
#include "Audio.h"
#include "FrameRateController.h"
#include <chrono>

SceneManager& SceneManager::Get()
{
    static SceneManager instance;
    return instance;
}

void SceneManager::LoadScene(const std::string& path)
{
    // Queue the scene load (do NOT load now)
    if (GetTransitionProgress() < 1.f) return;
    pendingScene = path;
}
void SceneManager::QueueLoadScene(const std::string& path)
{
    if (GetTransitionProgress() < 1.f) return;
    pendingScene = path;
}
void SceneManager::Update(float dt, float, int numSteps)
{
    
    if (Input::IsKeyPressed(GLFW_KEY_P) && IsPlaying()) {
        TogglePause();
        //std::cout << (sm.IsPaused() ? "[AudioManager] Game paused (music continues)\n" : "[AudioManager] Game resumed\n");
    }

    //if (Input::IsKeyPressed(GLFW_KEY_H) && IsPlaying()) {
    //    sceneTransitionTime = 0.f;
    //    sceneFadeDuration = 2.f;
    //    QueueLoadScene("../Assets/saves/level1.json");
    //    //std::cout << (sm.IsPaused() ? "[AudioManager] Game paused (music continues)\n" : "[AudioManager] Game resumed\n");
    //}

    // Only load scenes at SAFE time (start of frame)
    if (!pendingScene.empty())
    {
        if (GetTransitionProgress() < 1.f) {

            std::cout << "fadeprog " << GetTransitionProgress() << '\n';
            

            sceneTransitionTime += dt;
            return; 
        }
        sceneFadeDuration = 0.f;
        sceneTransitionTime = 0.f;


        std::string path = pendingScene;
        pendingScene.clear();

        // *** SAFE LOAD HERE ***
        JsonLoad::ClearAllEntities();

        if (JsonLoad::LoadScene(path))
        {
            currentScenePath = path;

#ifndef NO_IMGUI
            ImGuiSystem::UpdateSpawnCamera();
#else
            EnterPlayMode(); //this works but can someone factcheck if this is the right way to do
#endif
            /*Camera& cam = Camera::GetInstance();
            cam.SetMode(Camera::Mode::GAME);*/

            auto p = std::filesystem::path(path);
            tempScenePath = "../Assets/saves/temp.json";
            // ApplySceneDefaults();
            // EnterPlayMode();
            std::cout << "[SceneManager] Loaded scene: " << path << "\n";
        }
        else
        {
            std::cerr << "[SceneManager] FAILED to load scene: " << path << "\n";
        }
    }

    // ============================================================
    // HANDLE GAME PAUSE (P key) - ONLY IN PLAY MODE
    // ============================================================
    if (Input::IsKeyPressed(GLFW_KEY_P) && IsPlaying() && !IsInEditMode()) {
        TogglePause();
        std::cout << (IsPaused() ? "[SceneManager] Game paused\n" : "[SceneManager] Game resumed\n");
    }
}


void SceneManager::SaveScene(const std::string& path, const SavedScene::SavedCamera& camSettings)
{
    std::string sceneName = std::filesystem::path(path).stem().string();
    JsonLoad::SaveCurrentScene(path, sceneName, camSettings);
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
   

    LogicSystem::Get().Initialize();

    std::cout << "[SceneManager] Entered PLAY MODE.\n";
}

void SceneManager::EnterEditMode()
{
    isPlaying = false;
    isPaused = true;
    inEditMode = true;

    // STOP ALL AUDIO WHEN ENTERING EDIT MODE
    AudioSystem::StopAll();

    Camera& cam = Camera::GetInstance();
    cam.SetMode(Camera::Mode::EDITOR);
    //cam.ResetToCenter();

    std::cout << "[SceneManager] Entered EDIT MODE.\n";
}

std::string SceneManager::GetCurrentSceneName() const
{
    if (currentScenePath.empty())
        return "";

    return std::filesystem::path(currentScenePath).stem().string();
}
void SceneManager::ApplySceneDefaults()
{
    Render& renderer = Render::GetInstance();
    Camera& cam = Camera::GetInstance();

    // CAMERA DEFAULTS
    cam.SetMode(Camera::Mode::EDITOR);
    cam.SetZoom(1.0f);
    cam.ResetToCenter();


    cam.SetBounds(Vec2{ 0.0f, 0.0f }, Vec2{ 1920.0f, 1080.0f });

    // RENDERER DEFAULTS
    renderer.useCamera = true;
    renderer.UseVirtualWindow(true);
}
