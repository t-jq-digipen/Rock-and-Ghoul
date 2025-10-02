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
CoreEngine::CoreEngine() : running(true), lastFrame(0.0) {}
FrameRateController frameRate(60);

GSM gsm;
float g_dt; // Global dt

void CoreEngine::AddSystem(ISystem* sys) {
    systems.push_back(sys);
}

void CoreEngine::Initialize() {
    for (auto s : systems) s->Initialize();
    gsm.Init(GameState::GS_MAIN_MENU);

}

void CoreEngine::Run() {
    std::cout << "EngRun\n";
    double lastTitleUpdate = glfwGetTime();
    int frames = 0;
    while (running && !glfwWindowShouldClose(GLApp::ptr_window) && GSM_CurrentState() != GameState::GS_QUIT) {
        if (gsm.GetCurrentState() != GameState::GS_RESTART)
        {
            // --- 1. Bind function pointers for the new current state
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
            float dt = static_cast<float>(frameRate.GetDeltaTime());
            g_dt = std::clamp(dt, 0.0f, 0.05f);
            
            // 1. Update game state logic first
            gsm.CallUpdate();

            // 2. Draw background/UI (this calls glClear)
            gsm.CallDraw();

            //loop through all systems managed by the engine
            for (auto s : systems) {
                //high-resolution clock
                using clock = std::chrono::high_resolution_clock;
                // Record the starting timestamp before running the system update
                auto start = clock::now();
                // Call the system's update function, passing delta time for this frame
                s->Update(g_dt);
                // Record the ending timestamp immediately after update finishes
                auto end = clock::now();
                //how long the update took in milliseconds
                double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
                // ISystem* s = new PhysicsSystem();
                //*s dereferences the pointer, so typeid(*s) looks at the dynamic type of the object (PhysicsSystem),
                // returns a const char* that represents the type’s name.
                // Get the system's type name (like "class PhysicsSystem") as a string
                std::string sysName = typeid(*s).name();
                timings[sysName].ms = elapsedMs;

                // call Render() immediately after Update()
                if (auto* imguiSystem = dynamic_cast<ImGuiSystem*>(s)) {
                    imguiSystem->Render();
                }
            }

            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
                GSM_SetNextState(GameState::GS_QUIT);

            Input::Update();
            glfwSwapBuffers(GLApp::ptr_window);
            frameRate.EndFrame();

            frames++;
            double now = glfwGetTime();
            if (now - lastTitleUpdate >= 1.0) {
                float fps = frames / (now - lastTitleUpdate);

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
    for (int i = (int)systems.size() - 1; i >= 0; --i) {
        systems[i]->Shutdown();
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
const GameState& GSM_PreviousState()    { return gsm.GetPreviousState(); }
const GameState& GSM_CurrentState()     { return gsm.GetCurrentState(); }
const GameState& GSM_NextState()        { return gsm.GetNextState(); }
// ----------------------------------------


// SETTERS
// ----------------------------------------
void GSM_SetPreviousState(GameState state)  { gsm.SetPreviousState(state); }
void GSM_SetCurrentState(GameState state)   { gsm.SetCurrentState(state); }
void GSM_SetNextState(GameState state)      { gsm.SetNextState(state); }
// ----------------------------------------

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