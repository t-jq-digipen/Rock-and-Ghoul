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

#ifndef NO_IMGUI
#include "ImguiSystem.h"
#endif
#include "FrameRateController.h"
#include "DataDefs.h"
#include "MovementSystem.h"
#include <nlohmann/json.hpp>
#include "RenderSystem.h"
#include "GuiSystem.h"
#include "PlayerSystem.h"
#include "EnemySystem.h"
#include "PhysicsSystem.h"

#include "CollisionManager.h"

#include "BehaviourComponent.h"
#include "GameScripts.h"
#include "AudioComponent.h"
#include "MonoSystem.h"
#include "MonoScriptComponent.h"
#include "SceneManager.h"
#include "../src/rectpack2D/finders_interface.h"
CoreEngine::CoreEngine()
    : systems{}
    , timings{}
    , running(true)
    , lastFrame(0.0)
{
}
void CoreEngine::AddUpdateSystem(ISystem* sys)
{
    updateSystems.push_back(sys);
}

void CoreEngine::AddRenderSystem(ISystem* sys)
{
    renderSystems.push_back(sys);
}

// Target FPS
//FrameRateController frameRate(60);

// Legacy globals (still used in some systems like LogicSystem / scripts)
//GSM   gsm;
float g_dt;           // Global fixed dt
bool  g_gamePaused = true;

void CoreEngine::AddSystem(ISystem* sys)
{
    systems.push_back(sys);
}

void CoreEngine::Initialize()
{
    // ----------------------------------------------------
    // Register Systems (order matters!)
    // ----------------------------------------------------
    //AddSystem(&LogicSystem::Get());
    //AddSystem(new MovementSystem());
    //AddSystem(new PlayerSystem());
    ////AddSystem(new ForcesManager());
    //AddSystem(new EnemySystem());
    //AddSystem(new PhysicsSystem());
    //AddSystem(new RenderSystem());
    //AddSystem(&GuiSystem::Get());
    //AddSystem(new ImGuiSystem());
    //AddSystem(&MonoSystem::Get());

    AddUpdateSystem(&LogicSystem::Get());
    AddUpdateSystem(&MonoSystem::Get());
    // AddUpdateSystem(&AudioManager::Get());
    AddUpdateSystem(new MovementSystem());
    AddUpdateSystem(new PlayerSystem());
    AddUpdateSystem(new EnemySystem());
    AddUpdateSystem(new PhysicsSystem());


    AddRenderSystem(new RenderSystem());
    AddRenderSystem(&GuiSystem::Get());
#ifndef NO_IMGUI
    AddRenderSystem(new ImGuiSystem());
#endif
    AddRenderSystem(&AudioManager::Get()); // audio updates in both editor and play mode

    
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

    registry.Register_Component<ColliderComponent>("Collider", { "Transform" });
    std::cout << "REGISTRY | REGISTERED COLLIDER COMPONENT\n";

    registry.Register_Component<AudioComponent>("AudioComponent");
    std::cout << "REGISTRY | REGISTERED AUDIO COMPONENT\n";

    registry.Register_Component<HealthComponent>("Health");
    std::cout << "REGISTRY | REGISTERED HEALTH COMPONENT\n";

    //registry.Register_Component<InputComponent>("Input");
    //std::cout << "REGISTRY | REGISTERED INPUT COMPONENT\n";

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
    for (auto* s : updateSystems) s->Initialize();
    for (auto* s : renderSystems)  s->Initialize();


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
    FrameRateController frameRate(60);
    SceneManager& sm = SceneManager::Get();
    m_fpsLastUpdate = glfwGetTime();
    m_fpsFrames = 0;

    while (running && !glfwWindowShouldClose(GLApp::ptr_window))
    {
        frameRate.StartFrame();
        glfwPollEvents();

        float dt = static_cast<float>(frameRate.GetDeltaTime());
        dt = std::clamp(dt, 0.0f, 0.05f); // clamp variable dt

        float fixedDt = static_cast<float>(frameRate.GetFixedDeltaTime());
        int   numSteps = frameRate.GetCurrentNumberOfSteps();

       
        sm.Update(dt, fixedDt, numSteps);

        // expose fixed dt to scripts / systems that still read g_dt
        g_dt = fixedDt;

        updateTimings.clear();
        renderTimings.clear();

        for (auto* s : updateSystems)
            updateTimings[typeid(*s).name()] = 0.0f;

        for (auto* s : renderSystems)
            renderTimings[typeid(*s).name()] = 0.0f;

     
        using clock = std::chrono::high_resolution_clock;

        if (sm.IsPaused() || sm.IsEditor())
        {
            fixedDt = 0.f;
        }

        if (sm.IsPlaying() && !sm.IsPaused())
        {
            for (int step = 0; step < numSteps; ++step)
            {
                for (auto* s : updateSystems)
                {
                    auto start = clock::now();

                    s->Update(fixedDt);

                    auto end = clock::now();
                    double ms = DurationMs(start, end);

                    std::string name = typeid(*s).name();
                    updateTimings[name] += ms;   // accumulate into stable entry
                }
            }

            Update_AudioComponents(fixedDt);
        }


        float renderDt = fixedDt * numSteps;

        for (auto* s : renderSystems)
        {
            auto start = clock::now();

            s->Update(renderDt);

            auto end = clock::now();
            double ms = DurationMs(start, end);

            std::string name = typeid(*s).name();
            renderTimings[name] += ms;
        }


#ifndef NO_IMGUI
        if (ImGuiSystem::guiEnabled) {
            Render::GetInstance().EndSceneCapture();
        }

        // Now render ImGui (which displays the captured scene)
        for (auto* s : renderSystems)
        {
            if (auto* imgui = dynamic_cast<ImGuiSystem*>(s))
                imgui->Render();
        }
#endif
        


        // Input needs a frame-end update
        Input::Update();

        // Present frame
        glfwSwapBuffers(GLApp::ptr_window);
        frameRate.EndFrame();

        // ---------------------------------------------
        // FPS in window title
        // ---------------------------------------------
        m_fpsFrames++;
        double now = glfwGetTime();

        if (now - m_fpsLastUpdate >= 1.0)
        {
            m_fps = m_fpsFrames / (now - m_fpsLastUpdate);

            // optional: keep updating window title
            std::string title = "Rock & Ghoul - FPS: " + std::to_string(static_cast<int>(m_fps));
            glfwSetWindowTitle(GLApp::ptr_window, title.c_str());

            m_fpsFrames = 0;
            m_fpsLastUpdate = now;
        }

    }
}

void CoreEngine::Shutdown()
{
    GLApp::gameInitialized = false;
    //SceneManager::Get().UnloadCurrentScene();
    Registry::Get().Remove_AllEntities();
    // Shut down update systems in reverse
    for (int i = (int)updateSystems.size() - 1; i >= 0; --i)
    {
        ISystem* s = updateSystems[i];
        s->Shutdown();

        if (s != &GuiSystem::Get() &&
            s != &LogicSystem::Get() &&
            s != &MonoSystem::Get())
        {
            delete s;
        }
    }

    // Shut down render systems in reverse
    for (int i = (int)renderSystems.size() - 1; i >= 0; --i)
    {
        ISystem* s = renderSystems[i];
        s->Shutdown();

        if (s != &GuiSystem::Get() &&
            s != &AudioManager::Get()) // only delete heap ones
        {
            delete s;
        }
    }

    updateSystems.clear();
    renderSystems.clear();
}


void CoreEngine::Quit()
{
    AudioSystem::PlayButtonSound(AudioSystem::ButtonType::QUIT);

    double startTime = glfwGetTime();
    while (glfwGetTime() - startTime < 0.5) {
        AudioSystem::Update();
    }
    running = false;
}
