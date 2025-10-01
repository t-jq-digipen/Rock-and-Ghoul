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
#include <crtdbg.h>
void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
    if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
}
int main() {
    if (!GLApp::init(1920, 1080, "Rock & Ghoul")) return -1;
    EnableMemoryLeakChecking();

    CoreEngine& engine = CoreEngine::GetInstance();
    // Render& renderer = Render::GetInstance();

    // Create audio manager
    AudioManager* audio = new AudioManager();
    engine.AddSystem(audio);
    MainMenu::SetAudioManager(audio); // BUTTON CLICK SOUND

    engine.AddSystem(new PlayerSystem());
    engine.AddSystem(new PhysicsSystem());
    engine.AddSystem(new ImGuiSystem()); // ImGuiSystem* gets added to systems vector
    engine.AddSystem(new RenderSystem());

    // initialize all systems (this sets up FMOD, OpenGL, etc.)
    engine.Initialize();

    std::cout << "GETS PAST ENGINE INIT\n";

    engine.Run();
    std::cout << "GETS PAST ENGINE RUN\n";

    engine.Shutdown();
    Render::GetInstance().Shutdown();
    ImGuiSystem::ShutdownImGui();
    GLApp::cleanup();
    return 0;
}