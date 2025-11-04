#include "LogicSystem.h"
#include "BehaviourComponent.h"
#include "Registry.h"
#include <iostream>
#include "CoreEngine.h"
extern GSM gsm;
void LogicSystem::Initialize()
{
    std::cout << "[LogicSystem] Initialized" << std::endl;
}

void LogicSystem::Update(float dt)
{
  /*  auto& reg = Registry::Get();

    for (auto& [e, bc] : reg.Get_ComponentMap<BehaviourComponent>()) {
        if (bc.scriptName.empty()) continue;

        if (BehaviourFCT* f = GetBehaviour(bc.scriptName)) {
            if (!bc.started) {
                f->Init(e);
                bc.started = true;
            }
            f->Update(e, dt);
        }
    }*/
    if (gsm.isPaused || gsm.inEditMode)
        return;
    Registry& reg = Registry::Get();

    for (auto& [entity, bc] : reg.Get_ComponentMap<BehaviourComponent>())
    {
        for (ScriptID scriptID : bc.scriptIDList)
        {
            std::string scriptName;
            // keep scriptName synced with current ID (in case edited from ImGui)
            if (scriptID != ScriptID::NONE)
            {
                const auto& idToName = m_idToName;
                auto it = idToName.find(scriptID);
                if (it != idToName.end())
                    scriptName = it->second;
            }

            BehaviourFCT* f = GetBehaviour(scriptName);
            if (!f) continue;

            // detect runtime script change
            //static std::unordered_map<Entity, ScriptID> previousScript;
            //if (previousScript[entity] != scriptID)
            //{
            //    // if changed, restart Init()
            //    previousScript[entity] = scriptID;
            //    bc.startedScripts[scriptID] = false;
            //    std::cout << "[LogicSystem] Script changed at runtime on entity "
            //        << entity << " -> " << scriptName << std::endl;
            //}

            // normal script execution
            if (!bc.startedScripts[scriptID])
            {
                f->Init(entity);
                bc.startedScripts[scriptID] = true;
            }

            f->Update(entity, dt);
        }
        //// keep scriptName synced with current ID (in case edited from ImGui)
        //if (bc.scriptID != ScriptID::NONE)
        //{
        //    const auto& idToName = m_idToName;
        //    auto it = idToName.find(bc.scriptID);
        //    if (it != idToName.end())
        //        bc.scriptName = it->second;
        //}

        //BehaviourFCT* f = GetBehaviour(bc.scriptName);
        //if (!f) continue;

        //// detect runtime script change
        //static std::unordered_map<Entity, ScriptID> previousScript;
        //if (previousScript[entity] != bc.scriptID)
        //{
        //    // if changed, restart Init()
        //    previousScript[entity] = bc.scriptID;
        //    bc.started = false;
        //    std::cout << "[LogicSystem] Script changed at runtime on entity "
        //        << entity << " -> " << bc.scriptName << std::endl;
        //}

        //// normal script execution
        //if (!bc.started)
        //{
        //    f->Init(entity);
        //    bc.started = true;
        //}

        //f->Update(entity, dt);
    }

}
//void LogicSystem::Update(float dt)
//{
//    Registry& reg = Registry::Get();
//
//    for (auto& [entity, bc] : reg.Get_ComponentMap<BehaviourComponent>())
//    {
//        if (bc.scriptNames.empty()) continue;
//
//        // Ensure startedStates matches number of scripts
//        if (bc.startedStates.size() < bc.scriptNames.size())
//            bc.startedStates.resize(bc.scriptNames.size(), false);
//
//        for (size_t i = 0; i < bc.scriptNames.size(); ++i)
//        {
//            const std::string& scriptName = bc.scriptNames[i];
//            bool started = bc.startedStates[i];
//
//            BehaviourFCT* fct = GetBehaviour(scriptName);
//            if (!fct) continue; // skip if not found
//
//            if (!started)
//            {
//                fct->Init(entity);
//                started = true;
//            }
//
//            fct->Update(entity, dt);
//
//            // Save started flag back
//            bc.startedStates[i] = started;
//        }
//    }
//}

void LogicSystem::Shutdown()
{
    std::cout << "[LogicSystem] Shutting down..." << std::endl;
    m_behaviours.clear();
}

void LogicSystem::RegisterScript(const std::string& name, const BehaviourFCT& script, ScriptID id)
{
    m_behaviours[name] = script;
    m_nameToID[name] = id;
    m_idToName[id] = name;
    std::cout << "[LogicSystem] Registered script: " << name
        << " (ID=" << static_cast<unsigned int>(id) << ")\n";
}
BehaviourFCT* LogicSystem::GetBehaviourByID(ScriptID id)
{
    auto it = m_idToName.find(id);
    if (it == m_idToName.end()) return nullptr;
    return GetBehaviour(it->second);
}
BehaviourFCT* LogicSystem::GetBehaviour(const std::string& name)
{
    auto it = m_behaviours.find(name);
    return (it != m_behaviours.end()) ? &it->second : nullptr;
}