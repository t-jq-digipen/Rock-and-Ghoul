// CoreEngine.h
#pragma once
#include <vector>
#include "System.h"

#include "Registry.h"
#include "GameStateManager.h"

class CoreEngine {
    std::vector<ISystem*> systems;
    bool running;
    double lastFrame;

    // Systems (e.g physics/render/transform yada yada)

public:
    CoreEngine();

    void AddSystem(ISystem* sys);
    void Initialize();
    void Run();
    void Shutdown();
    void Quit();

};




// GSM functions

// STATE GETTERS
// ----------------------------------------
const GameState& GSM_PreviousState();
const GameState& GSM_CurrentState();
const GameState& GSM_NextState();
// ----------------------------------------

// SETTERS
// ----------------------------------------
void GSM_SetPreviousState(GameState state);
void GSM_SetCurrentState(GameState state);
void GSM_SetNextState(GameState state);
// ----------------------------------------

// DEBUGGER
std::ostream& operator<<(std::ostream& os, const GameState& state);