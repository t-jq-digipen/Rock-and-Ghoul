#include "CoreEngine.h"
#include "System.h"
#include "glapp.h"
#include "Input.h"
#include <GLFW/glfw3.h>

CoreEngine::CoreEngine() : running(true), lastFrame(0.0) {}

void CoreEngine::AddSystem(ISystem* sys) {
    systems.push_back(sys);
}

void CoreEngine::Initialize() {
    for (auto s : systems) s->Initialize();
    lastFrame = glfwGetTime();
}

void CoreEngine::Run() {
    while (running && !glfwWindowShouldClose(GLApp::ptr_window)) {
        double now = glfwGetTime();
        float dt = static_cast<float>(now - lastFrame);
        lastFrame = now;

        glfwPollEvents();
        
        for (auto s : systems) s->Update(dt);
        Input::Update();
        glfwSwapBuffers(GLApp::ptr_window);
    }
}

void CoreEngine::Shutdown() {
    for (int i = (int)systems.size() - 1; i >= 0; --i) {
        systems[i]->Shutdown();
        delete systems[i];
    }
    systems.clear();
}

void CoreEngine::Quit() {
    running = false;
}
