//manager for all game systems
#pragma once
#include "Render.h"
#include "BossGhoul.h"
#include "Spells.h"
#include "Fretboard.h"
#include "RenderSystem.h" 

class GameContext {
private:
    static GameContext* instance;

public:
    // Core systems
    Render renderer;
    SpellManager spellManager;
    BossGhoulSystem bossGhoulSystem;
    RenderSystem* renderSystem = nullptr;
    // Level-specific objects
    FretBoard* currentFretboard = nullptr;

    // Textures and resources
    GLuint bgTexture = 0;

    // Singleton access
    static GameContext& Get() {
        if (!instance) {
            instance = new GameContext();
        }
        return *instance;
    }

    static void Initialize() {
        Get(); // Creates instance if needed
    }

    static void Cleanup() {
        delete instance;
        instance = nullptr;
    }

    // Constructor
    GameContext() : currentFretboard(nullptr) {
        // Initialize fretboard with spellManager reference
        currentFretboard = new FretBoard(0, 0, 800, 200, spellManager);
        renderSystem = new RenderSystem(renderer);
    }

    ~GameContext() {
        delete currentFretboard;
        delete renderSystem;
    }

    // Prevent copying
    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;
};

// Static member definition (put this in GameContext.cpp)
// GameContext* GameContext::instance = nullptr;