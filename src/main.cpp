/* ========================================================
 * @file        main.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Manfred Sim         | manfred.sim@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief       
 *
 * The main entry point for the application. Enables checks
 * for memory leaks and installs a crash handler before starting
 * up the application. When the application starts, it creates
 * a 1920x1080 game window and initializes the core engine
 * needed to run the game, as well as the following core systems:
 * 
 * - Audio Manager (for managing audio files)
 * - Render System (for rendering shape and sprite components)
 * - Player System (for handling player behaviour)
 * - Physics System (for handling in-game physics and collisions)
 * - ImGui System (for implementing game editor)
 *
 * Cleans up all resources once the application is closed.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "CoreEngine.h"
#include "AudioSystem.h"  // contains AudioManager
#include "MainMenu.h"
#include "RenderSystem.h"
#include "PlayerSystem.h"
#include "PhysicsSystem.h"
#include "ImguiSystem.h"
#include "CrashHandler.h"

#include "JsonLoad.h"
#include "FilePath.h"

#include "MonoManager.h"
#include <crtdbg.h>

void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
}

int main() {
    // Load settings from file FIRST (before creating window)
    auto settingsOpt = JsonLoad::LoadSettings(FilePath::SETTINGS_FILE);

    GameSettings settings;
    if (settingsOpt.has_value()) {
        settings = settingsOpt.value();
        std::cout << "Loaded settings from file" << std::endl;
    }
    else {
        // If file doesn't exist, create default settings
        settings.volume = 100.0f;
        settings.resolutionWidth = 1920;
        settings.resolutionHeight = 1080;
        settings.fullscreen = false;

        // Save defaults for next time
        JsonLoad::SaveSettings(FilePath::SETTINGS_FILE, settings);
    }

    // USE the loaded settings to initialize window
    if (!GLApp::init(settings.resolutionWidth, settings.resolutionHeight, "Rock & Ghoul")) {
        return -1;
    }
    CrashHandler::SetLogPath("rockandghoul_crash.log");
    CrashHandler::Install();
    CrashHandler::Log("=== Rock & Ghoul starting ===");
    EnableMemoryLeakChecking();

    int rc = CrashHandler::RunProtected([&]
        {
            //if (!GLApp::init(1920, 1080, "Rock & Ghoul"))
            //{
            //    CrashHandler::Log("GLApp::init failed");
            //    return -1;
            //}
            CoreEngine& engine = CoreEngine::GetInstance();
            //Render& renderer = Render::GetInstance();

            // Create audio manager
            AudioManager* audio = new AudioManager();
            engine.AddSystem(audio);
            MainMenu::SetAudioManager(audio); // BUTTON CLICK SOUND

            engine.AddSystem(new RenderSystem());
            engine.AddSystem(new PlayerSystem());
            engine.AddSystem(new PhysicsSystem());
            engine.AddSystem(new ImGuiSystem()); // ImGuiSystem* gets added to systems vector

            // initialize all systems (FMOD, OpenGL, etc.)
            engine.Initialize();
            CrashHandler::Log("Engine initialized");
            std::cout << "GETS PAST ENGINE INIT\n";

            MonoManager& mono = MonoManager::Get();
            mono.Initialize("../Assets/Scripts/GameScripts.dll");
            float dt = 0.016f;
            mono.InvokeMethod("Rotator", "Update", dt);
            mono.InvokeMethod("Mover", "Update", dt);
            engine.Run();
            CrashHandler::Log("Engine run returned");
            std::cout << "GETS PAST ENGINE RUN\n";

            engine.Shutdown();
            CrashHandler::Log("Engine shutdown complete");
            mono.Shutdown();
            return 0;
        });

    // ---- explicit cleanup order ----
    Render::GetInstance().Shutdown();   // free GPU resources
    GLApp::cleanup();                   // destroy GLFW + GL context
    CrashHandler::Uninstall();          //crash handler last

    return rc;
}
