/* ========================================================
 * @file        CoreEngine.cpp
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Manfred Sim         | manfred.sim@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date        01 NOV 2025
 *
 * @brief   Core Engine implementation and update loop
 *
 * The CoreEngine class is responsible for running the main
 * loop of the game.
 *
 * The engine:
 * - Initializes various systems and the Game State Manager(GSM).
 * - Controls frame rate and delta-time clamping.
 * - Manages system updates and timing measurements for tracking
 *   system performances
 * - Handles rendering, input management, and gamestate transitions.
 * - Shuts down all subsystems.
 *
 * The GSM has global functions provided for interfacing, allowing
 * retrieval and modification of current, previous, and next states
 * outside of the engine code.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "CoreEngine.h"
#include "System.h"
#include "glapp.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include "GameStateManager.h"
#include <iostream>
#include <string>
#include "ImguiSystem.h"
#include "FrameRateController.h"
#include <algorithm>
#include "DataDefs.h"
#include "MovementSystem.h"
#include <nlohmann/json.hpp>
#include "RenderSystem.h"
#include "GuiSystem.h"

#include "PlayerSystem.h"
#include "EnemySystem.h"
#include "PhysicsSystem.h"
#include "MonoSystem.h"

#include "BehaviourComponent.h"
#include "GameScripts.h"
#include "AudioComponent.h"

#include "MonoScriptComponent.h"
CoreEngine::CoreEngine() : systems{}, timings{}, running(true), lastFrame(0.0) {}
FrameRateController frameRate(60);

GSM gsm;
float g_dt; // Global dt
bool g_gamePaused = false; // pause state flag

void CoreEngine::AddSystem(ISystem* sys) {
    systems.push_back(sys);
}

void CoreEngine::Initialize() {
    // --- SYSTEMS ---

    AddSystem(&LogicSystem::Get());
    AddSystem(new MovementSystem());
    AddSystem(new PlayerSystem());
    //AddSystem(new ForcesManager());
    AddSystem(new EnemySystem());
    AddSystem(new PhysicsSystem());
    AddSystem(new RenderSystem());
    AddSystem(&GuiSystem::Get());
    AddSystem(new ImGuiSystem());
    AddSystem(new MonoSystem());
    // --- SYSTEMS END ---

    std::cout << "[ImGui] GLApp::ptr_window = " << GLApp::ptr_window
        << " at " << (void*)&GLApp::ptr_window << std::endl;

    // --- COMPONENTS ---
    Registry& registry = Registry::Get();
    std::cout << "\n\nINITIALIZING REGISTRY\n==============================\n";
    std::cout << "INITIALIZING DEFAULT COMPONENTS: \n==============================\n";
    registry.Register_Component<TransformComponent>("Transform");
    std::cout << "REGISTRY | REGISTERED TRANSFORM COMPONENT\n";

    registry.Register_Component<Sprite::SpriteComponent>("Sprite", { "Transform" });
    std::cout << "REGISTRY | REGISTERED SPRITE COMPONENT\n";

    registry.Register_Component<Animator::AnimatorComponent>("Animator", { "Transform", "Sprite" });
    std::cout << "REGISTRY | REGISTERED ANIMATOR COMPONENT\n";
    
    registry.Register_Component<Physics::PhysicsComponent>("Physics", { "Transform" });
    std::cout << "REGISTRY | REGISTERED PHYSICS COMPONENT\n";

    registry.Register_Component<AudioComponent>("Audio");
    std::cout << "REGISTRY | REGISTERED AUDIO COMPONENT\n";

    registry.Register_Component<HealthComponent>("Health");
    std::cout << "REGISTRY | REGISTERED HEALTH COMPONENT\n";

    registry.Register_Component<InputComponent>("Input");
    std::cout << "REGISTRY | REGISTERED INPUT COMPONENT\n";

    registry.Register_Component<ShapeComponent>("Shape", { "Transform" });
    std::cout << "REGISTRY | REGISTERED SHAPE COMPONENT\n";

    registry.Register_Component<Movement::MovementComponent>("Movement", { "Transform", "Physics" });
    std::cout << "REGISTRY | REGISTERED MOVEMENT COMPONENT\n";

    // marker for non-standard components which should be shifted out to game files
    std::cout << "NON DEFAULT COMPONENTS: \n==============================\n";

    registry.Register_Component<TagComponent>("Tag");
    std::cout << "REGISTRY | REGISTERED TAG COMPONENT\n";
    /*registry.Register_Component<ElementComponent>("Element");
    std::cout << "REGISTRY | REGISTERED ELEMENT COMPONENT\n";*/
    registry.Register_Component<BehaviourComponent>("Behaviour");
    //Registry::Get().Register_Component<>("MonoScriptComponent");
    registry.Register_Component<MonoScriptComponent>("MonoScriptComponent");

    registry.Register_Component<RenderLayerComponent>("RenderLayer");
    std::cout << "REGISTRY | REGISTERED RENDERLAYER COMPONENT\n";
    // --- COMPONENTS END ---

    for (auto s : systems) s->Initialize();
    gsm.Init(GameState::GS_MAIN_MENU);
    GLApp::gameInitialized = true;
    //RegisterAllGameScripts();
}

void CoreEngine::Run() {
    std::cout << "EngRun\n";
    double lastTitleUpdate = glfwGetTime();
    int frames = 0;
    while (running && !glfwWindowShouldClose(GLApp::ptr_window) && GSM_CurrentState() != GameState::GS_QUIT) {
        if (gsm.GetCurrentState() != GameState::GS_RESTART)
        {
            // --- 1. Bind function pointers for the 
            // current state
            gsm.Update();

            // --- 2. Load + Init ONCE for this state
            gsm.CallLoad();
        }
        else
        {
            gsm.Restart();
        }

        gsm.CallInit();

        // --- 3. Frame loop for this state
        while (gsm.GetNextState() == gsm.GetCurrentState()) {
            frameRate.StartFrame();

            glfwPollEvents();

            // getting actual frame time
            float dt = static_cast<float>(frameRate.GetDeltaTime());
            dt = std::clamp(dt, 0.0f, 0.05f);

            // getting fixed timestep info (fixed dt variable)
            float fixedDt = static_cast<float>(frameRate.GetFixedDeltaTime());
            int numSteps = frameRate.GetCurrentNumberOfSteps();

            // fixed dt proof to console
            static bool lagSimulationActive = false;
            if (Input::IsKeyPressed(GLFW_KEY_L)) {
                lagSimulationActive = !lagSimulationActive;
                if (lagSimulationActive) {
                    std::cout << "\n[FIXED DT] LAG TEST STARTED\n";
                }
                else {
                    std::cout << "[FIXED DT] LAG TEST STOPPED\n\n";
                }
            }

            if (lagSimulationActive) {
                // busy wait loop
                double endTime = glfwGetTime() + 0.05;  // Wait 50ms
                while (glfwGetTime() < endTime) {
                    // busy waiting
                }

                static int frameCounter = 0;
                if (++frameCounter % 30 == 0) {
                    std::cout << "[LAG] Frame DT: " << dt << "s | Physics Steps: " << numSteps
                        << " | Fixed DT: " << fixedDt << "s" << std::endl;
                }
            }

            // Save real dt for audio (music should always play even in edit mode)
            float audioDt = dt;

            // Handle play/stop flags
            if (gsm.isPaused && gsm.inEditMode)
            {
                // In edit mode freeze simulation
                fixedDt = 0.0f;
                numSteps = 0;  // No physics updates in edit mode
            }

            // Set global dt to fixed timestep for deterministic simulation
            g_dt = fixedDt;

            // --- 1. Update logic ---
            gsm.CallUpdate();

            // --- 2. Capture game view for ImGui ---
            Render& renderer = Render::GetInstance();
            renderer.BeginSceneCapture();

            // Draw the current game scene or main menu
            gsm.CallDraw();




            // Finish capture � ImGui will display this texture inside "Game View"
            renderer.EndSceneCapture();
            // NEED TO FIGURE OUT HOW TO SPLIT THIS PER DRAW/UPDATE

            //loop through all systems managed by the engine
            for (auto s : systems) {
                //high-resolution clock
                using clock = std::chrono::high_resolution_clock;
                // Record the starting timestamp before running the system update
                auto start = clock::now();
                // Call the system's update function, passing delta time for this frame
                if (!gsm.isPaused && !gsm.inEditMode)
                {
                    // Only update systems when in gameplay
                    s->Update(fixedDt);  // Changed from g_dt to fixedDt
                }
                else
                {
                    //Still render ImGui overlay while editing
                    if (auto* imguiSystem = dynamic_cast<ImGuiSystem*>(s))
                        imguiSystem->Update(0.0f);
                    else if (auto* renderSystem = dynamic_cast<RenderSystem*>(s))
                        renderSystem->Update(0.0f);
                    else if (auto* physicsSystem = dynamic_cast<Physics::PhysicsComponent*>(s))
                        physicsSystem->Update(0.0f);
                    else if (auto* audioManager = dynamic_cast<AudioManager*>(s))
                        audioManager->Update(audioDt);  // use real dt for audio
                    // GuiSystem::Get().Update(0.0f);
                }
                // Record the ending timestamp immediately after update finishes
                auto end = clock::now();
                //how long the update took in milliseconds
                double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
                // ISystem* s = new PhysicsSystem();
                //*s dereferences the pointer, so typeid(*s) looks at the dynamic type of the object (PhysicsSystem),
                // returns a const char* that represents the type's name.
                // Get the system's type name (like "class PhysicsSystem") as a string
                std::string sysName = typeid(*s).name();
                timings[sysName].ms = elapsedMs;

                // call Render() immediately after Update()
                if (auto* imguiSystem = dynamic_cast<ImGuiSystem*>(s)) {
                    renderer.EndSceneCapture();
                    imguiSystem->Render();
                }
            }
            gsm.CallDraw();
            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
                GSM_SetNextState(GameState::GS_QUIT);

            Input::Update();
            glfwSwapBuffers(GLApp::ptr_window);
            frameRate.EndFrame();

            frames++;
            double now = glfwGetTime();
            if (now - lastTitleUpdate >= 1.0) {
                double fps = frames / (now - lastTitleUpdate);

                std::string title = "Rock & Ghoul - FPS: " + std::to_string((int)fps);
                glfwSetWindowTitle(GLApp::ptr_window, title.c_str());

                frames = 0;
                lastTitleUpdate = now;
            }
        }


        // --- 4. Clean up the old state
        gsm.CallFree();
        if (gsm.GetNextState() != GameState::GS_RESTART) {
            gsm.CallUnload();
        }

        // --- 5. Actually switch state
        gsm.ChangeNextState();
    }
}

void CoreEngine::Shutdown() {
    GLApp::gameInitialized = false;
    for (int i = (int)systems.size() - 1; i >= 0; --i) {
        systems[i]->Shutdown();
        if (systems[i] != &GuiSystem::Get() && systems[i] != &LogicSystem::Get())
            delete systems[i];
    }
    systems.clear();
    gsm.Close();   // To exit the game state manager

}

void CoreEngine::Quit() {
    running = false;
}

// GAME STATE MANAGER
// ================================================================================

// STATE GETTERS
// ----------------------------------------
const GameState& GSM_PreviousState() { return gsm.GetPreviousState(); }
const GameState& GSM_CurrentState() { return gsm.GetCurrentState(); }
const GameState& GSM_NextState() { return gsm.GetNextState(); }
const bool GSM_IsPaused() { return gsm.IsPaused(); }
const bool GSM_InEditMode() { return gsm.InEditMode(); }
// ----------------------------------------


// SETTERS
// ----------------------------------------
void GSM_SetPreviousState(GameState state) { gsm.SetPreviousState(state); }
void GSM_SetCurrentState(GameState state) { gsm.SetCurrentState(state); }
void GSM_SetNextState(GameState state) { gsm.SetNextState(state); }
void GSM_SetIsPaused(bool isPaused) { gsm.SetPaused(isPaused); }
void GSM_SetInEditMode(bool inEditMode) { gsm.SetEditMode(inEditMode); }
// ----------------------------------------

// PAUSE STATE
// ----------------------------------------
bool GSM_IsGamePaused() { return g_gamePaused; }
void GSM_SetGamePaused(bool paused) { g_gamePaused = paused; }

// DEBUGGER
std::ostream& operator<<(std::ostream& os, const GameState& state)
{
    switch (state)
    {
    case GameState::GS_MAIN_MENU:
    {
        os << "Main Menu";
        break;
    }
    case GameState::GS_LEVEL1:
    {
        os << "Level 1";
        break;
    }
    case GameState::GS_RESTART:
    {
        os << "Restart";
        break;
    }
    case GameState::GS_QUIT:
    {
        os << "Quit";
        break;
    }
    }
    return os;
}