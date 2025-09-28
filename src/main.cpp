#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glapp.h"
#include "CoreEngine.h"
#include "AudioSystem.h"  // contains AudioManager
#include "RenderSystem.h"
#include "PlayerSystem.h"
#include "BossGhoul.h"
#include "PhysicsSystem.h"
// testing json
#include "JsonLoad.h"
#include "FilePath.h"
#include <iostream>

// ADD: Global audio pointers for GSM to use
AudioManager* audio = nullptr;        // CHANGED: Make global
FMOD::Sound* menuMusic = nullptr;     // ADD: Main menu music
FMOD::Sound* gameMusic = nullptr;     // ADD: Game level music

BossGhoulSystem* bossGhoulSystem = nullptr;

int main() {
    // json testing
    GameSettings settings;
    settings.volume = 10.0f;
    settings.resolutionHeight = 1080;
    settings.resolutionWidth = 1920;

    if (JsonLoad::SaveSettings(FilePath::SETTINGS_FILE, settings)) {
        std::cout << "Settings saved successfully yippee!" << std::endl;
    }
    else { std::cout << "IT FAILED BOOOOOO" << std::endl; }

    // loading it
    auto loaded = JsonLoad::LoadSettings(FilePath::SETTINGS_FILE);
    if (loaded) {
        std::cout << "Settings loaded. Volume: " << loaded->volume << std::endl;
    }
    else { std::cout << "Fail" << std::endl; }
    // end of json testing

    if (!GLApp::init(1920, 1080, "Rock & Ghoul")) return -1;

    CoreEngine engine;
    Render& renderer = Render::GetInstance();

    // CHANGED: Create audio system and assign to global pointer
    audio = new AudioManager();  // Now assigns to global 'audio'
    engine.AddSystem(audio);

    bossGhoulSystem = new BossGhoulSystem();
    engine.AddSystem(bossGhoulSystem);
    engine.AddSystem(new RenderSystem(renderer));
    engine.AddSystem(new PlayerSystem());
    engine.AddSystem(new PhysicsSystem(Render::GetInstance()));

    // initialize all systems (this sets up FMOD, OpenGL, etc.)
    engine.Initialize();

    std::cout << "GETS PAST ENGINE INIT\n";

    // CHANGED: Load both music tracks for crossfading
    menuMusic = audio->Load("../Assets/music/sniffmyBalls.wav", true);  // ADD: Load menu music
    gameMusic = audio->Load("../Assets/music/sniffmyBallsAgain.wav", true);  // ADD: Load game music

    // CHANGED: Play menu music initially (not bgm)
    if (menuMusic) {
        audio->PlayMusic(menuMusic, false);
        std::cout << "Started menu music\n";
    }
    else {
        std::cout << "Failed to load menu music\n";
    }

    std::cout << "GETS PAST ENGINE RUN\n";

    engine.Run();
    engine.Shutdown();

    // ADD: Cleanup audio resources
    if (audio) {
        if (menuMusic) audio->Unload(menuMusic);
        if (gameMusic) audio->Unload(gameMusic);
        delete audio;
        audio = nullptr;
    }

    GLApp::cleanup();
    return 0;
}