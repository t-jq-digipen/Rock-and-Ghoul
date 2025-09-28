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

// --- New: Inline GameLogicSystem class ---
class GameLogicSystem : public ISystem {
    AudioManager* audio;
public:
    GameLogicSystem(AudioManager* audioMgr) : audio(audioMgr) {}

    void Initialize() override {}

    void Update(float dt) override {
        if (Input::IsKeyPressed(GLFW_KEY_C)) {
            static bool triggered = false;
            if (!triggered) {
                std::cout << "[Debug] Attempting to load new track...\n";

                FMOD::Sound* newTrack = audio->Load("../Assets/music/sniffmyBallsAgain.wav", true);
                if (!newTrack) {
                    std::cout << "[Error] Failed to load new track!\n";
                    return;
                }
                std::cout << "[Debug] Loaded new track: " << newTrack << "\n";

                // CrossfadeTo returns void, so do NOT assign its result
                audio->CrossfadeTo(newTrack, 5.0f);
                std::cout << "Crossfade started from GameLogicSystem!\n";

                triggered = true;
            }
        }
    }



    void Shutdown() override {}
};
// --- End new GameLogicSystem ---

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

    // create 1 audio system and keep a pointer to it
    AudioManager* audio = new AudioManager();
    engine.AddSystem(audio);

    // --- New: create and add the GameLogicSystem here ---
    GameLogicSystem* logicSystem = new GameLogicSystem(audio);
    engine.AddSystem(logicSystem);
    // --- End new ---

    bossGhoulSystem = new BossGhoulSystem();
    engine.AddSystem(bossGhoulSystem);
    engine.AddSystem(new RenderSystem(renderer));
    engine.AddSystem(new PlayerSystem());
    engine.AddSystem(new PhysicsSystem(Render::GetInstance()));

    // initialize all systems (this sets up FMOD, OpenGL, etc.)
    engine.Initialize();

    std::cout << "GETS PAST ENGINE INIT\n";
    std::cout << "GETS PAST ENGINE RUN\n";

    //load and play sounds
    FMOD::Sound* bgm = audio->Load("../Assets/music/sniffmyBalls.wav", true);
    audio->Play(bgm);

    engine.Run();
    engine.Shutdown();

    GLApp::cleanup();
    return 0;
}
