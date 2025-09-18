// CoreEngine.h
#pragma once
#include <vector>
#include "System.h"

class CoreEngine {
    std::vector<ISystem*> systems;
    bool running;
    double lastFrame;

public:
    CoreEngine();
    void AddSystem(ISystem* sys);
    void Initialize();
    void Run();
    void Shutdown();
    void Quit();
};
