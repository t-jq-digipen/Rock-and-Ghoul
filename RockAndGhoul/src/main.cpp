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

// Single include for all engine systems.
#include "reginegine.h"

// RockAndGhoul game-specific systems
#include "PlayerSystem.h"
#include "EnemySystem.h"
#include "CombatSystem.h"

void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
}

int main(int argc, char* argv[]) {
#ifdef NO_IMGUI
    // Shipping build: always load RockAndGhoul directly.
    FilePath::SetGameRoot("RockAndGhoul");
#else
    // Editor build: if a game name was passed as argv[1], load it directly.
    // Otherwise open the game-picker popup.
    if (argc > 1 && argv[1] && argv[1][0] != '\0') {
        FilePath::SetGameRoot(argv[1]);
    } else {
        FilePath::InitRepoRoot();
    }
#endif

    // Load settings from file (only valid once a game root is set)
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
            if (!FilePath::SETTINGS_FILE.empty())
                JsonLoad::SaveSettings(FilePath::SETTINGS_FILE, settings);
        }
    }
    else {
        // If file doesn't exist, create default settings
        settings.volume = 100.0f;
        settings.resolutionWidth = 1920;
        settings.resolutionHeight = 1080;
        // Editor starts windowed so the game-picker popup is visible
        settings.fullscreen = !FilePath::SETTINGS_FILE.empty();

        // Save defaults for next time (only when a game path is set)
        if (!FilePath::SETTINGS_FILE.empty())
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

            // Register RockAndGhoul-specific systems before Initialize().
            // These run between MovementSystem and PhysicsSystem.
            engine.RegisterGameSystem(new PlayerSystem());
            engine.RegisterGameSystem(new EnemySystem());
            engine.RegisterGameSystem(&CombatSystem::Get(), false); // singleton — don't delete

            // Register RockAndGhoul-specific behaviour scripts.
            // LogicSystem::Initialize() will call this after engine-level scripts are registered.
            // To add a new script: write a namespace with Start/Update/End, then add it here.
            LogicSystem::Get().SetScriptRegistrar([]()
            {
                LogicSystem& ls = LogicSystem::Get();
                {
                    BehaviourFCT player;
                    player.Init   = [](Entity e)           { PlayerMove::Start(e); };
                    player.Update = [](Entity e, float dt) { PlayerMove::Update(e, dt); };
                    player.End    = [](Entity e)            { PlayerMove::End(e); };
                    ls.RegisterScript("PlayerMovement", player, "RockAndGhoul");
                }
                {
                    BehaviourFCT rock;
                    rock.Init   = [](Entity e)           { RockScript::Start(e); };
                    rock.Update = [](Entity e, float dt) { RockScript::Update(e, dt); };
                    rock.End    = [](Entity e)            { RockScript::End(e); };
                    ls.RegisterScript("RockScript", rock, "RockAndGhoul");
                }
                {
                    BehaviourFCT tutorial;
                    tutorial.Init   = [](Entity e)           { TutorialScript::Start(e); };
                    tutorial.Update = [](Entity e, float dt) { TutorialScript::Update(e, dt); };
                    tutorial.End    = [](Entity e)            { TutorialScript::End(e); };
                    ls.RegisterScript("TutorialScript", tutorial, "RockAndGhoul");
                }
            });

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
