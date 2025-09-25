#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "CoreEngine.h"
#include "AudioSystem.h"  // contains AudioManager
#include "RenderSystem.h"
#include "PlayerSystem.h"
#include "Singleton.h"


int main() {
    if (!GLApp::init(1920, 1080, "Rock & Ghoul")) return -1;

    CoreEngine engine;
    GameContext::Initialize();
    GameContext& ctx = GameContext::Get();
    // create 1 audio system and keep a pointer to it
    AudioManager* audio = new AudioManager();
    engine.AddSystem(audio);

    engine.AddSystem(&ctx.bossGhoulSystem);
    engine.AddSystem(new RenderSystem(ctx.renderer));
    engine.AddSystem(new PlayerSystem());

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

    GameContext::Cleanup();
    GLApp::cleanup();
    return 0;
}
