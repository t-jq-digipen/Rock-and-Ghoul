#include "CoreEngine.h"
#include "System.h"
#include "glapp.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include "GameStateManager.h"
#include <iostream>
#include <string>

CoreEngine::CoreEngine() : running(true), lastFrame(0.0) {}

Registry registry;

GSM gsm;

void CoreEngine::AddSystem(ISystem* sys) {
    systems.push_back(sys);
}

void CoreEngine::Initialize() {
    for (auto s : systems) s->Initialize();
    lastFrame = glfwGetTime();
    gsm.Init(GameState::GS_MAIN_MENU);

}

void CoreEngine::Run() {
    std::cout << "EngRun\n";
    while (running && !glfwWindowShouldClose(GLApp::ptr_window) && GSM_CurrentState() != GameState::GS_QUIT) {
        // Game State Manager

        // Load a game state
        if (gsm.GetCurrentState() != GameState::GS_RESTART)
        {

            std::cout << "EngRunning, state = " << (GSM_CurrentState()) << std::endl;
            gsm.Update();       // Set the function pointers to the current game state
            std::cout << "Got past update\n";
            gsm.CallLoad();     // Call the current game state's Load() function
            std::cout << "Got past load\n";
        }
        // Restart the current game state
        else
        {
            gsm.Restart();
        }
        gsm.CallInit(); // Call the current game state's Initialize() function

        // While a game state is running
        while (gsm.GetNextState() == gsm.GetCurrentState())
        {
            double now = glfwGetTime();
            float dt = static_cast<float>(now - lastFrame);
            lastFrame = now;

            glfwPollEvents();
            for (auto s : systems) s->Update(dt);
            gsm.CallUpdate(); // Call the current game state's Update() function
            gsm.CallDraw();   // Call the current game state's Draw() function

            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) GSM_SetNextState(GameState::GS_QUIT);              // quit program (shortcut)


            Input::Update();
            glfwSwapBuffers(GLApp::ptr_window);

        }
        gsm.CallFree();   // Call the current game state's Free() function

        // When the game state has changed, unload the current game state
        if (gsm.GetNextState() != GameState::GS_RESTART)
        {
            gsm.CallUnload(); // Call the current game state's Unload() function
        }
        // Change the game state (by setting nextState)
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

// GSM functions

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