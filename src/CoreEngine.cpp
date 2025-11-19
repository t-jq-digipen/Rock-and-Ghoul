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
 * - Initializes all engine systems.
 * - Controls frame rate and delta-time clamping.
 * - Manages system updates and timing measurements for tracking
 *   system performance.
 * - Handles rendering, input management, and scene transitions
 *   through SceneManager.
 * - Shuts down all subsystems.
 *
 * GSM is kept only as a legacy global while older code still
 * references it, but scene flow is now driven by SceneManager.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "CoreEngine.h"
#include "System.h"
#include "glapp.h"
#include "Input.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <typeinfo>


#include "ImguiSystem.h"
#include "FrameRateController.h"
#include "DataDefs.h"
#include "MovementSystem.h"
#include <nlohmann/json.hpp>
#include "RenderSystem.h"
#include "GuiSystem.h"
#include "PlayerSystem.h"
#include "EnemySystem.h"
#include "PhysicsSystem.h"
#include "BehaviourComponent.h"
#include "GameScripts.h"
#include "AudioComponent.h"
#include "MonoSystem.h"
#include "MonoScriptComponent.h"
#include "SceneManager.h"

CoreEngine::CoreEngine()
    : systems{}
    , timings{}
    , running(true)
    , lastFrame(0.0)
{
}

// Target FPS
FrameRateController frameRate(60);

// Legacy globals (still used in some systems like LogicSystem / scripts)
//GSM   gsm;
float g_dt;           // Global fixed dt
bool  g_gamePaused = false;

void CoreEngine::AddSystem(ISystem* sys)
{
    systems.push_back(sys);
}

void CoreEngine::Initialize()
{
    // ----------------------------------------------------
    // Register Systems (order matters!)
    // ----------------------------------------------------
    AddSystem(&LogicSystem::Get());
    AddSystem(new MovementSystem());
    AddSystem(new PlayerSystem());
    //AddSystem(new ForcesManager());
    AddSystem(new EnemySystem());
    AddSystem(new PhysicsSystem());
    AddSystem(new RenderSystem());
    AddSystem(&GuiSystem::Get());
    AddSystem(new ImGuiSystem());
    AddSystem(&MonoSystem::Get());

    std::cout << "[ImGui] GLApp::ptr_window = " << GLApp::ptr_window
        << " at " << (void*)&GLApp::ptr_window << std::endl;

    // ----------------------------------------------------
    // Register Components
    // ----------------------------------------------------
    Registry& registry = Registry::Get();
    std::cout << "\n\nINITIALIZING REGISTRY\n==============================\n";
    std::cout << "INITIALIZING DEFAULT COMPONENTS: \n==============================\n";

    registry.Register_Component<TransformComponent>("Transform");
    std::cout << "REGISTRY | REGISTERED TRANSFORM COMPONENT\n";

    // Old sprite / animator / shape chain replaced by RenderComponent
    //registry.Register_Component<Sprite::SpriteComponent>("Sprite", { "Transform" });
    //registry.Register_Component<Animator::AnimatorComponent>("Animator", { "Transform", "Sprite" });
    //registry.Register_Component<ShapeComponent>("Shape", { "Transform" });

    // New render pipeline
    registry.Register_Component<RenderComponent>("Render", { "Transform" });
    std::cout << "REGISTRY | REGISTERED RENDER COMPONENT\n";

    registry.Register_Component<Physics::PhysicsComponent>("Physics", { "Transform" });
    std::cout << "REGISTRY | REGISTERED PHYSICS COMPONENT\n";

    registry.Register_Component<AudioComponent>("Audio");
    std::cout << "REGISTRY | REGISTERED AUDIO COMPONENT\n";

    registry.Register_Component<HealthComponent>("Health");
    std::cout << "REGISTRY | REGISTERED HEALTH COMPONENT\n";

    registry.Register_Component<InputComponent>("Input");
    std::cout << "REGISTRY | REGISTERED INPUT COMPONENT\n";

    registry.Register_Component<Movement::MovementComponent>("Movement", { "Transform", "Physics" });
    std::cout << "REGISTRY | REGISTERED MOVEMENT COMPONENT\n";

    registry.Register_Component<LifetimeComponent>("Lifetime");
    std::cout << "REGISTRY | REGISTERED LIFETIME COMPONENT\n";

    std::cout << "NON DEFAULT COMPONENTS: \n==============================\n";

    registry.Register_Component<TagComponent>("Tag");
    std::cout << "REGISTRY | REGISTERED TAG COMPONENT\n";

    registry.Register_Component<BehaviourComponent>("Behaviour");
    std::cout << "REGISTRY | REGISTERED BEHAVIOUR COMPONENT\n";

    registry.Register_Component<MonoScriptComponent>("MonoScriptComponent");
    std::cout << "REGISTRY | REGISTERED MONOSCRIPT COMPONENT\n";

    registry.Register_Component<RenderLayerComponent>("RenderLayer");
    std::cout << "REGISTRY | REGISTERED RENDERLAYER COMPONENT\n";

    // ----------------------------------------------------
    // Initialize all systems
    // ----------------------------------------------------
    for (auto* s : systems)
        s->Initialize();

    // ----------------------------------------------------
    // Initial scene
    // ----------------------------------------------------
    SceneManager& sm = SceneManager::Get();

    // Load your default scene (main menu)
    sm.LoadScene("../Assets/saves/MainMenu.json");

    // Legacy GSM init – can be removed once all GSM users are gone
    //gsm.Init(GameState::GS_MAIN_MENU);

    // Flag for ImGui / editor that engine is ready
    GLApp::gameInitialized = true;
}

void CoreEngine::Run()
{
    std::cout << "EngRun\n";

    double lastTitleUpdate = glfwGetTime();
    int    frames = 0;

    while (running && !glfwWindowShouldClose(GLApp::ptr_window))
    {
        frameRate.StartFrame();
        glfwPollEvents();

        float dt = static_cast<float>(frameRate.GetDeltaTime());
        dt = std::clamp(dt, 0.0f, 0.05f); // clamp variable dt

        float fixedDt = static_cast<float>(frameRate.GetFixedDeltaTime());
        int   numSteps = frameRate.GetCurrentNumberOfSteps();

        // Let SceneManager do any book-keeping if it ever needs to
        SceneManager& sm = SceneManager::Get();
        sm.Update(dt, fixedDt, numSteps);

        // expose fixed dt to scripts / systems that still read g_dt
        g_dt = fixedDt;

        // ------------------------------------------------
        // Update all systems
        // ------------------------------------------------
        for (auto* s : systems)
        {
            using clock = std::chrono::high_resolution_clock;
            auto start = clock::now();

            // 1. RenderSystem ALWAYS runs
            if (auto* render = dynamic_cast<RenderSystem*>(s))
            {
                render->Update(fixedDt);
            }
            // 2. ImGuiSystem ALWAYS runs (so you can change scenes, press play, etc.)
            else if (auto* imgui = dynamic_cast<ImGuiSystem*>(s))
            {
                imgui->Update(dt);
            }
            // 3. All other systems only when actually playing
            else
            {
                if (sm.IsPlaying() && !sm.IsPaused())
                {
                    s->Update(fixedDt);
                }
            }

            auto   end = clock::now();
            double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();

            std::string sysName = typeid(*s).name();
            timings[sysName].ms = elapsedMs;

            // ImGui render pass MUST happen after all systems
            if (auto* imguiSystem = dynamic_cast<ImGuiSystem*>(s))
            {
                imguiSystem->Render();
            }
        }

        // Input needs a frame-end update
        Input::Update();

        // Present frame
        glfwSwapBuffers(GLApp::ptr_window);
        frameRate.EndFrame();

        // ---------------------------------------------
        // FPS in window title
        // ---------------------------------------------
        frames++;
        double now = glfwGetTime();
        if (now - lastTitleUpdate >= 1.0)
        {
            double fps = frames / (now - lastTitleUpdate);
            std::string title = "Rock & Ghoul - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(GLApp::ptr_window, title.c_str());
            frames = 0;
            lastTitleUpdate = now;
        }
    }
}

void CoreEngine::Shutdown()
{
    GLApp::gameInitialized = false;

    // Shut down in reverse order; delete only heap-allocated ones
    for (int i = static_cast<int>(systems.size()) - 1; i >= 0; --i)
    {
        ISystem* s = systems[i];
        s->Shutdown();

        if (s != &GuiSystem::Get() &&
            s != &LogicSystem::Get() &&
            s != &MonoSystem::Get())
        {
            delete s;
        }
    }

    systems.clear();
}

void CoreEngine::Quit()
{
    running = false;
}
