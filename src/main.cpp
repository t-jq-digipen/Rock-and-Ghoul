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

int main() {
    if (!GLApp::init(1920, 1080, "Rock & Ghoul")) return -1;
    CoreEngine& engine = CoreEngine::GetInstance();
    Render& renderer = Render::GetInstance();
    // Create audio manager
    AudioManager* audio = new AudioManager();
    engine.AddSystem(audio);
    MainMenu::SetAudioManager(audio); //BUTTON CLICK SOUND
    engine.AddSystem(new RenderSystem(renderer));
    engine.AddSystem(new PlayerSystem());
    engine.AddSystem(new PhysicsSystem());

    engine.AddSystem(new ImGuiSystem()); // ImGuiSystem* gets added to systems vector
    // initialize all systems (this sets up FMOD, OpenGL, etc.)
    engine.Initialize();
    // broken here?
    std::cout << "GETS PAST ENGINE INIT\n";

    engine.Run();
    std::cout << "GETS PAST ENGINE RUN\n";

    //load and play sounds
    FMOD::Sound* bgm = audio->Load("../Assets/music/bgm.mp3", true);
    audio->Play(bgm);

    engine.Shutdown();

    GLApp::cleanup();
    return 0;
}
