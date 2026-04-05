/* ========================================================
 * @file        CoreEngine.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu 30%
 * @co-author   Manfred Sim         | manfred.sim@digipen.edu 30%
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu 40%
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

    int m_fpsFrames = 0;
    double m_fpsLastUpdate = 0.0;
    double m_fps = 0.0;

    struct TimingInfo { double ms = 0.0; };

    double DurationMs(const std::chrono::high_resolution_clock::time_point& start,
        const std::chrono::high_resolution_clock::time_point& end) const
    {
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
    std::vector<ISystem*> updateSystems;
    std::vector<ISystem*> renderSystems;

    std::unordered_map<std::string, double> updateTimings;
    std::unordered_map<std::string, double> renderTimings;

    // Game specific systems registered before Initialize() in RegisterGameSystem()
    struct GameSystemEntry { 
        ISystem* sys; 
        bool ownsMemory; 
    
    };
    std::vector<GameSystemEntry> m_pendingGameSystems;

    // Systems (e.g physics/render/transform yada yada)
    CoreEngine();
public:
    //for in game perf imgui
    static CoreEngine& GetInstance() {
        static CoreEngine instance;
        return instance;
    }
    void AddUpdateSystem(ISystem* sys);
    void AddRenderSystem(ISystem* sys);

    const std::unordered_map<std::string, double>& GetUpdateTimings() const { return updateTimings; }
    const std::unordered_map<std::string, double>& GetRenderTimings()  const { return renderTimings; }

    // Adds systems to the Core Engine
    void AddSystem(ISystem* sys);

    // Call before Initialize() to inject game specific update systems.
    // They run between MovementSystem and PhysicsSystem.
    //   engine.RegisterGameSystem(new PlayerSystem());          // engine owns + deletes
    //   engine.RegisterGameSystem(&CombatSystem::Get(), false); // singleton, don't delete
    void RegisterGameSystem(ISystem* sys, bool ownsMemory = true);

    // Initialize the Core Engine
    void Initialize();

    // Run the Core Engine
    void Run();

    // Shutdown the Core Engine
    void Shutdown();

    // Quit the Core Engine
    void Quit();

    double GetFPS() const { return m_fps; }
    //getter function for your timings map
    //the key is a std::string
    //the value is a SystemTiming struct (contains ms for elapsed milliseconds).
    std::unordered_map<std::string, SystemTiming>& GetTimings() { return timings; }
};




//// GSM functions
//
//// STATE GETTERS
//// ----------------------------------------
//const GameState& GSM_PreviousState();
//const GameState& GSM_CurrentState();
//const GameState& GSM_NextState();
//const bool GSM_IsPaused();
//const bool GSM_InEditMode();
//// ----------------------------------------
//
//// SETTERS
//// ----------------------------------------
//void GSM_SetPreviousState(GameState state);
//void GSM_SetCurrentState(GameState state);
//void GSM_SetNextState(GameState state);
//void GSM_SetIsPaused(bool isPaused);
//void GSM_SetInEditMode(bool inEditMode);
//// ----------------------------------------
//
//// PAUSE STATE FUNCTIONS
//// ----------------------------------------
//// wannna pause all of gameplay where it is. when state changes entirely it'll unload so pause needs to have its own thing otherwise it'll unload and reload level each time
//bool GSM_IsGamePaused();
//void GSM_SetGamePaused(bool paused);
//
//// DEBUGGER
//std::ostream& operator<<(std::ostream& os, const GameState& state);