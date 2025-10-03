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
    ImGuiSystem::ShutdownImGui();       // 1. destroy ImGui (needs GL context alive)
    Render::GetInstance().Shutdown();   // 2. free GPU resources
    GLApp::cleanup();                   // 3. finally destroy GLFW + GL context
    CrashHandler::Uninstall();          // 4. crash handler last

    return rc;
}
