#pragma once
#include "System.h"
#include "MonoManager.h"
#include "Registry.h"
#include "MonoScriptComponent.h"
#include <iostream>

class MonoSystem : public ISystem
{
public:
    void Initialize() override
    {
        std::cout << "[MonoSystem] Initializing Mono runtime...\n";
        MonoManager::Get().Initialize("../Assets/Scripts/GameScripts.dll");
    }

    void Update(float dt) override
    {
        Registry& reg = Registry::Get();
        auto& scriptMap = reg.Get_ComponentMap<MonoScriptComponent>();

        // Loop through all entities that have MonoScriptComponent
        for (auto& [entity, script] : scriptMap)
        {
            MonoManager::Get().InvokeMethod(script.className, "Update", dt);
        }
    }

    void Shutdown() override
    {
        MonoManager::Get().Shutdown();
    }
};
