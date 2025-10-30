/* ========================================================
 * @file        CoreEngine.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Manfred Sim         | manfred.sim@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief   Core Engine declaration
 *
 * Declares the CoreEngine class and interface functions for
 * managing game loop, along with system management and
 * Game State Manager (GSM) integration.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <vector>
#include "System.h"

#include "Registry.h"
#include "GameStateManager.h"
#include <chrono>
#include <unordered_map>
#include <string>

extern float g_dt;

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

    // Adds systems to the Core Engine
    void AddSystem(ISystem* sys);

    // Initialize the Core Engine
    void Initialize();

    // Run the Core Engine
    void Run();

    // Shutdown the Core Engine
    void Shutdown();

    // Quit the Core Engine
    void Quit();

    //getter function for your timings map
    //the key is a std::string
    //the value is a SystemTiming struct (contains ms for elapsed milliseconds).
    std::unordered_map<std::string, SystemTiming>& GetTimings() { return timings; }
};




// GSM functions

// STATE GETTERS
// ----------------------------------------
const GameState& GSM_PreviousState();
const GameState& GSM_CurrentState();
const GameState& GSM_NextState();
const bool GSM_IsPaused();
const bool GSM_InEditMode();
// ----------------------------------------

// SETTERS
// ----------------------------------------
void GSM_SetPreviousState(GameState state);
void GSM_SetCurrentState(GameState state);
void GSM_SetNextState(GameState state);
void GSM_SetIsPaused(bool isPaused);
void GSM_SetInEditMode(bool inEditMode);
// ----------------------------------------

// DEBUGGER
std::ostream& operator<<(std::ostream& os, const GameState& state);