#pragma once
#include <vector>
#include <memory>
#include "Input.h"
#include "Audio.h"
#include "Render.h"
#include "Player.h"

class Engine {
private:
    // Core systems
    Render renderer;
    bool isRunning;
    double lastFrameTime;
    float deltaTime;

    // Game objects
    std::vector<Player> players;

    // Resources
    GLuint fretboardTexture;
    FMOD::Sound* backgroundMusic;

    // Window settings
    int windowWidth;
    int windowHeight;
    const char* windowTitle;

public:
    Engine(int width = 2400, int height = 1350, const char* title = "Rock & Ghoul");
    ~Engine();

    // Core engine functions
    bool Initialize();
    void Run();
    void Shutdown();

    // Game loop functions
    void Update(float dt);
    void Render();
    void HandleInput();

    // Game object management
    void AddPlayer(float x, float y, ControlScheme controls);
    void UpdatePlayers(float dt);
    void CheckCollisions();

    // Resource management
    bool LoadResources();
    void UnloadResources();

    // Getters
    bool IsRunning() const { return isRunning; }
    float GetDeltaTime() const { return deltaTime; }
};