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

int main() {
    CrashHandler::SetLogPath("rockandghoul_crash.log");
    CrashHandler::Install();
    CrashHandler::Log("=== Rock & Ghoul starting ===");

    const int rc = CrashHandler::RunProtected([&]
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

    CrashHandler::Logf("Exiting with code %d", rc);
    CrashHandler::Uninstall();
    return rc;
}