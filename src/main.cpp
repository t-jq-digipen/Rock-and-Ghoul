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
            return;
        }

        struct Cleanup
        {
            ~Cleanup() noexcept
            {
                GLApp::cleanup();
                CrashHandler::Log("GLApp cleaned up");
            }
        } cleanup_guard;

        CoreEngine& engine = CoreEngine::GetInstance();
        Render& renderer = Render::GetInstance();

        // Create audio manager
        AudioManager* audio = new AudioManager();
        engine.AddSystem(audio);
        MainMenu::SetAudioManager(audio); // BUTTON CLICK SOUND

        engine.AddSystem(new RenderSystem(renderer));
        engine.AddSystem(new PlayerSystem());
        engine.AddSystem(new PhysicsSystem());
        engine.AddSystem(new ImGuiSystem()); // ImGuiSystem* gets added to systems vector

   
    return 0;
        // initialize all systems (this sets up FMOD, OpenGL, etc.)
        engine.Initialize();
        CrashHandler::Log("Engine initialized");
        std::cout << "GETS PAST ENGINE INIT\n";

        engine.Run();
        CrashHandler::Log("Engine run returned");
        std::cout << "GETS PAST ENGINE RUN\n";

        engine.Shutdown();
        CrashHandler::Log("Engine shutdown complete");


    });
    Render::GetInstance().Shutdown();
    ImGuiSystem::ShutdownImGui();
    GLApp::cleanup();
    CrashHandler::Uninstall();
    return rc;
}