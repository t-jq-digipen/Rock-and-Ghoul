#include "CoreEngine.h"
#include "System.h"
#include "glapp.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include "GameStateManager.h"

CoreEngine::CoreEngine() : running(true), lastFrame(0.0) {}

void CoreEngine::AddSystem(ISystem* sys) {
    systems.push_back(sys);
}

void CoreEngine::Initialize() {
    for (auto s : systems) s->Initialize();
    lastFrame = glfwGetTime();
    GSM::Init(GS_MAIN_MENU);    // To initialize the game state manager
}

void CoreEngine::Run() {
    while (running && !glfwWindowShouldClose(GLApp::ptr_window) && currentState != GS_QUIT) {
        // Game State Manager

        // Load a game state
        if (currentState != GS_RESTART)
        {
            GSM::Update();  // Set the function pointers to the current game state
            fpLoad();   // Call the current game state's Load() function
        }
        // Restart the current game state
        else
        {
            nextState = previousState;
            currentState = previousState;
        }
        fpInitialize(); // Call the current game state's Initialize() function

        // While a game state is running
        while (nextState == currentState)
        {
            double now = glfwGetTime();
            float dt = static_cast<float>(now - lastFrame);
            lastFrame = now;

            glfwPollEvents();
            for (auto s : systems) s->Update(dt);
            fpUpdate(); // Call the current game state's Update() function
            fpDraw();   // Call the current game state's Draw() function

            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) nextState = GS_QUIT;              // quit program (shortcut)

            Input::Update();
            glfwSwapBuffers(GLApp::ptr_window);

        }
        fpFree();   // Call the current game state's Free() function

        // When the game state has changed, unload the current game state
        if (nextState != GS_RESTART)
        {
            fpUnload(); // Call the current game state's Unload() function
        }
        // Change the game state (by setting nextState)
        previousState = currentState;
        currentState = nextState;
    }
}

void CoreEngine::Shutdown() {
    for (int i = (int)systems.size() - 1; i >= 0; --i) {
        systems[i]->Shutdown();
        delete systems[i];
    }
    systems.clear();
    GSM::Close();   // To exit the game state manager
}

void CoreEngine::Quit() {
    running = false;
}
