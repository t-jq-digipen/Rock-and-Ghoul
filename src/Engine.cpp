#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Engine.h"
#include "glapp.h"
#include "Collision.h"
#include <iostream>


Engine::Engine(int width, int height, const char* title)
    : isRunning(false), lastFrameTime(0.0), deltaTime(0.0f),
    windowWidth(width), windowHeight(height), windowTitle(title),
    fretboardTexture(0), backgroundMusic(nullptr) {
}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize() {
    // Initialize audio system first
    if (!AudioSystem::Init()) {
        std::cout << "FMOD initialization failed!" << std::endl;
        return false;
    }

    // Initialize OpenGL context
    if (!GLApp::init(windowWidth, windowHeight, windowTitle)) {
        std::cout << "Unable to create OpenGL context" << std::endl;
        AudioSystem::Close();
        return false;
    }

    // Initialize renderer
    renderer.Init();

    // Load resources
    if (!LoadResources()) {
        std::cout << "Failed to load resources!" << std::endl;
        return false;
    }

    // Create players
    AddPlayer(200.0f, 200.0f, ControlScheme::WASD);
    AddPlayer(400.0f, 200.0f, ControlScheme::Arrows);

    // Start background music
    if (backgroundMusic) {
        AudioSystem::PlaySound(backgroundMusic);
    }

    lastFrameTime = glfwGetTime();
    isRunning = true;
    return true;
}

void Engine::Run() {
    if (!Initialize()) {
        std::cout << "Engine initialization failed!" << std::endl;
        return;
    }

    // Main game loop
    while (isRunning && !glfwWindowShouldClose(GLApp::ptr_window)) {
        double currentTime = glfwGetTime();
        deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        HandleInput();
        Update(deltaTime);
        Render();

        glfwSwapBuffers(GLApp::ptr_window);
    }

    Shutdown();
}

void Engine::Update(float dt) {
    // Update core systems
    GLApp::update_time(1.0);
    GLApp::update();
    Input::Update();
    AudioSystem::Update();

    // Update game objects
    UpdatePlayers(dt);
    CheckCollisions();
}

void Engine::Render() {
    // Clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw background elements (like fretboard)
    if (fretboardTexture) {
        renderer.DrawTexturedQuad({ 1200.0f, 200.0f }, { 1000.0f, 200.0f }, fretboardTexture);
    }

    // Draw all players
    for (auto& player : players) {
        player.Draw();
    }
}

void Engine::HandleInput() {
    glfwPollEvents();

    // Handle any global input here
    // Individual player input is handled in UpdatePlayers()
}

void Engine::UpdatePlayers(float dt) {
    // 1. Compute intended movement for all players
    for (auto& player : players) {
        player.ComputeInput(dt);
    }

    // 2. Apply movement (collision checking handled separately)
    for (auto& player : players) {
        player.ApplyMove();
    }
}

void Engine::CheckCollisions() {
    // Check collisions between all players
    for (size_t i = 0; i < players.size(); ++i) {
        for (size_t j = i + 1; j < players.size(); ++j) {
            if (CollisionIntersection_RectRect_Static(
                players[i].GetFutureAABB(),
                players[j].GetFutureAABB())) {

                std::cout << "Collision detected between players " << i << " and " << j << std::endl;
                players[i].CancelMove();
                players[j].CancelMove();
            }
        }
    }
}

void Engine::AddPlayer(float x, float y, ControlScheme controls) {
    players.emplace_back(x, y, controls);
}

bool Engine::LoadResources() {
    // Load textures
    fretboardTexture = Render::LoadTexture("../Assets/fretboard.jpg");
    if (fretboardTexture == 0) {
        std::cout << "Failed to load fretboard texture!" << std::endl;
        return false;
    }

    // Load audio
    backgroundMusic = AudioSystem::LoadSound("../Assets/music/bgm.mp3", true);
    if (!backgroundMusic) {
        std::cout << "Failed to load background music!" << std::endl;
        return false;
    }

    return true;
}

void Engine::UnloadResources() {
    // Cleanup textures
    if (fretboardTexture) {
        glDeleteTextures(1, &fretboardTexture);
        fretboardTexture = 0;
    }

    // Audio cleanup is handled by AudioSystem::Close()
}

void Engine::Shutdown() {
    if (!isRunning) return;

    UnloadResources();
    AudioSystem::Close();
    GLApp::cleanup();
    isRunning = false;
}