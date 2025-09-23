// CoreEngine.h
#pragma once
#include <vector>
#include "System.h"

#include "Registry.h"

class CoreEngine {
    std::vector<ISystem*> systems;
    bool running;
    double lastFrame;

    // ECS
    Registry registry;

    // Systems (e.g physics/render/transform yada yada)

public:
    CoreEngine();
    void AddSystem(ISystem* sys);
    void Initialize();
    void Run();
    void Shutdown();
    void Quit();
};
