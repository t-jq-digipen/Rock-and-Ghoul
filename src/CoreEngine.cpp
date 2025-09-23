#include "CoreEngine.h"
#include "System.h"
#include "glapp.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include "GameStateManager.h"

#include "Registry.h"

CoreEngine::CoreEngine() : running(true), lastFrame(0.0) {}

void CoreEngine::AddSystem(ISystem* sys) {
    systems.push_back(sys);
}

void CoreEngine::Initialize() {
    for (auto s : systems) s->Initialize();
    lastFrame = glfwGetTime();
    GSM::Init(GS_MAIN_MENU);

    Entity player = registry.AddEntity();
}

void CoreEngine::Run() {
    while (running && !glfwWindowShouldClose(GLApp::ptr_window) && currentState != GS_QUIT) {
        // Game State Manager
        if (currentState != GS_RESTART)
        {
            GSM::Update();
            fpLoad();
        }
        else
        {
            nextState = previousState;
            currentState = previousState;
        }
        fpInitialize();
        while (nextState == currentState)
        {
            double now = glfwGetTime();
            float dt = static_cast<float>(now - lastFrame);
            lastFrame = now;

            glfwPollEvents();
            for (auto s : systems) s->Update(dt);
            fpUpdate();
            fpDraw();

            if (currentState == GS_MAIN_MENU && Input::IsKeyPressed(GLFW_KEY_SPACE))
            {
                nextState = GS_LEVEL1;
            }

            if (currentState != GS_MAIN_MENU)
            {
                if (Input::IsKeyPressed(GLFW_KEY_1)) nextState = GS_LEVEL1;             // go to level 1 state
                //else if (Input::IsKeyPressed(GLFW_KEY_2)) nextState = GS_LEVEL2;        // go to level 2 state
                else if (Input::IsKeyPressed(GLFW_KEY_M)) nextState = GS_MAIN_MENU;     // go to main menu state
                else if (Input::IsKeyPressed(GLFW_KEY_R)) nextState = GS_RESTART;       // restart state
                else if (Input::IsKeyPressed(GLFW_KEY_Q)) nextState = GS_QUIT;          // quit program
            }

            Input::Update();
            glfwSwapBuffers(GLApp::ptr_window);

        }
        fpFree();
        if (nextState != GS_RESTART)
        {
            fpUnload();
        }
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
    GSM::Close();
}

void CoreEngine::Quit() {
    running = false;
}
