#include "LogicSystem.h"
#include "BehaviourComponent.h"
#include "Registry.h"
#include <iostream>

void LogicSystem::Initialize()
{
    std::cout << "[LogicSystem] Initialized" << std::endl;
}

void LogicSystem::Update(float dt)
{
    auto& reg = Registry::Get();

    for (auto& [e, bc] : reg.Get_ComponentMap<BehaviourComponent>()) {
        if (bc.scriptName.empty()) continue;

        if (BehaviourFCT* f = GetBehaviour(bc.scriptName)) {
            if (!bc.started) {
                f->Init(e);
                bc.started = true;
            }
            f->Update(e, dt);
        }
    }
}

void LogicSystem::Shutdown()
{
    std::cout << "[LogicSystem] Shutting down..." << std::endl;
    m_behaviours.clear();
}

void LogicSystem::RegisterScript(const std::string& name, const BehaviourFCT& script)
{
    m_behaviours[name] = script;
    std::cout << "[LogicSystem] Registered script: " << name << std::endl;
}

BehaviourFCT* LogicSystem::GetBehaviour(const std::string& name)
{
    auto it = m_behaviours.find(name);
    return (it != m_behaviours.end()) ? &it->second : nullptr;
}