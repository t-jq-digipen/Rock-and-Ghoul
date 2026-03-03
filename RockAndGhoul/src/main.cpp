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
 * - Logic System (for attaching logic scripts to entities)
 * - Movement System (for rendering movement component)
 * - Player System (for handling player behaviour)
 * - Enemy System (for handling enemy behaviour)
 * - Forces Manager (for applying forces onto entities)
 * - Physics System (for handling in-game physics and collisions)
 * - Render System (for rendering shape and sprite components)
 * - Gui System (for rendering GUI elements in the game)
 * - ImGui System (for implementing game editor)
 * - Mono System (for implementing mono scripts)
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
#include "RenderSystem.h"
#include "PlayerSystem.h"
#include "PhysicsSystem.h"
#include "ImguiSystem.h"
#include "CrashHandler.h"
#include "GuiSystem.h"
#include "MovementSystem.h"
#include "EnemySystem.h"

#include "JsonLoad.h"
#include "FilePath.h"

#include "MonoManager.h"
#include "MonoScriptComponent.h"
#include "MonoSystem.h"
#include <crtdbg.h>
//#include "ForcesManager.h"

#include "LogicSystem.h"
void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
}

int main() {
    // Set the active game folder FIRST so all FilePath members are valid.
    // For a new game project, change this to its folder name e.g. "MySecondGame".
    FilePath::SetGameRoot("RockAndGhoul");

    // Load settings from file FIRST (before creating window)
    auto settingsOpt = JsonLoad::LoadSettings(FilePath::SETTINGS_FILE);

    GameSettings settings;
    if (settingsOpt.has_value()) {
        try {
            settings = settingsOpt.value();
            std::cout << "Loaded settings from file" << std::endl;
        }
        catch (const nlohmann::json::type_error& e)
        {
            std::cerr << "Settings file is invalid: " << e.what() << std::endl;
            // Fallback to defaults
            settings.volume = 100.0f;
            settings.resolutionWidth = 1920;
            settings.resolutionHeight = 1080;
            settings.fullscreen = true;
            JsonLoad::SaveSettings(FilePath::SETTINGS_FILE, settings);
        }
    }
    else {
        // If file doesn't exist, create default settings
        settings.volume = 100.0f;
        settings.resolutionWidth = 1920;
        settings.resolutionHeight = 1080;
        settings.fullscreen = true;

        // Save defaults for next time
        JsonLoad::SaveSettings(FilePath::SETTINGS_FILE, settings);
    }

#ifdef NO_IMGUI
    settings.fullscreen = true;
#endif

    // USE the loaded settings to initialize window
    if (!GLApp::init(settings.resolutionWidth, settings.resolutionHeight, "Rock & Ghoul", settings.fullscreen)) {
        return -1;
    }

 
 

    const char* LogPath = "rockandghoul_crash.log";

    // Truncate the file at startup (creates it if missing)
    {
        std::ofstream trunc(LogPath, std::ios::out | std::ios::trunc);
        // trunc goes out of scope and closes immediately
    }
    
    CrashHandler::SetLogPath(LogPath);
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
            //AudioManager* audio = new AudioManager();
            //engine.AddSystem(audio);
            //MainMenu::SetAudioManager(audio); // BUTTON CLICK SOUND

            // initialize all systems (FMOD, OpenGL, etc.)
            engine.Initialize();
            CrashHandler::Log("Engine initialized");
            std::cout << "GETS PAST ENGINE INIT\n";

            //MonoManager& mono = MonoManager::Get();
            //mono.Initialize("../Assets/Scripts/
            // s.dll");

            //// Register the MonoScript component with the ECS registry
            //Registry::Get().Register_Component<MonoScriptComponent>("MonoScriptComponent");


            //// Add the MonoSystem to the engine so it runs every frame
            //engine.AddSystem(new MonoSystem());

            engine.Run();
            CrashHandler::Log("Engine run returned");
            std::cout << "GETS PAST ENGINE RUN\n";

            engine.Shutdown();
            GLApp::cleanup();
            CrashHandler::Log("Engine shutdown complete");
           // mono.Shutdown();
            return 0;
        });

    // ---- explicit cleanup order ----
    Render::GetInstance().Shutdown();   // free GPU resources
    //GLApp::cleanup();                   // destroy GLFW + GL context
    CrashHandler::Uninstall();          //crash handler last

    return rc;
}
