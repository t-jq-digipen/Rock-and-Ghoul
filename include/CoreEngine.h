// CoreEngine.h
#pragma once
#include <vector>
#include "System.h"

#include "Registry.h"
#include "GameStateManager.h"
#include <chrono>
#include <unordered_map>
#include <string>

struct SystemTiming {
    double ms = 0.0; // milliseconds per frame
};
class CoreEngine {
    std::vector<ISystem*> systems;
    std::unordered_map<std::string, SystemTiming> timings;
    bool running;
    double lastFrame;

    // Systems (e.g physics/render/transform yada yada)
    CoreEngine();
public:
    //for in game perf imgui
    static CoreEngine& GetInstance() {
        static CoreEngine instance;
        return instance;
    }

    void AddSystem(ISystem* sys);
    void Initialize();
    void Run();
    void Shutdown();
    void Quit();
    //getter function for your timings map
    //the key is a std::string (system name "PhysicsSystem"),
    //the value is a SystemTiming struct (contains ms for elapsed milliseconds).
    std::unordered_map<std::string, SystemTiming>& GetTimings() { return timings; }
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