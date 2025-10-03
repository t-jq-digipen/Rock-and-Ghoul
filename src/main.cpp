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
#include <crtdbg.h>

void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
}

int main() {
    CrashHandler::SetLogPath("rockandghoul_crash.log");
    CrashHandler::Install();
    CrashHandler::Log("=== Rock & Ghoul starting ===");
    EnableMemoryLeakChecking();

    int rc = CrashHandler::RunProtected([&]
        {
            if (!GLApp::init(1920, 1080, "Rock & Ghoul"))
            {
                CrashHandler::Log("GLApp::init failed");
                return -1;
            }
            CoreEngine& engine = CoreEngine::GetInstance();
            Render& renderer = Render::GetInstance();

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

            engine.Run();
            CrashHandler::Log("Engine run returned");
            std::cout << "GETS PAST ENGINE RUN\n";

            engine.Shutdown();
            CrashHandler::Log("Engine shutdown complete");

            return 0;
        });

    // ---- explicit cleanup order ----
    //ImGuiSystem::ShutdownImGui();       // 1. destroy ImGui (needs GL context alive)
    Render::GetInstance().Shutdown();   // 2. free GPU resources
    GLApp::cleanup();                   // 3. finally destroy GLFW + GL context
    CrashHandler::Uninstall();          // 4. crash handler last

    return rc;
}
