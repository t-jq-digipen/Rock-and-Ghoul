/* ========================================================
 * @file        LogicSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan       | t.junquan@digipen.edu
 * @date        07 NOV 2025
 *
 * @brief
 * Manages and updates all C++ behaviour scripts registered in the ECS.
 *
 * The LogicSystem serves as the gameplaY logic. It retrieves all entities
 * that contain a BehaviourComponent and executes their behaviour functions each frame.

 * Each BehaviourComponent maintains a list of script IDs
 * that correspond to registered logic routines stored in system’s behaviour map.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ======================================================== 
 */

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
    //each BehaviourComponent knows its entity ID

    for (auto& [entity, bc] : m_registry.Get_ComponentMap<BehaviourComponent>())
    {
        bc.FixID(entity);
    }

    //skip if paused or editing
    if (gsm.isPaused || gsm.inEditMode)
        return;

    //each entity with a behaviour component
    for (auto& [entity, bc] : m_registry.Get_ComponentMap<BehaviourComponent>())
    {
        for (ScriptID scriptID : bc.scriptIDList)
        {
            //find name from id
            std::string scriptName;
            if (scriptID != ScriptID::NONE)
            {
                const auto& idToName = m_idToName;
                auto it = idToName.find(scriptID);
                if (it != idToName.end())
                    scriptName = it->second;
            }
            //get behaviourfct
            BehaviourFCT* f = GetBehaviour(scriptName);
            if (!f) continue;

            
            if (!bc.startedScripts[scriptID])
            {
                //first time, call Init(entity)
                f->Init(entity);
                bc.startedScripts[scriptID] = true;
            }
            //after call update
            f->Update(entity, dt);
        }

    }

}

void LogicSystem::Shutdown()
{
    std::cout << "[LogicSystem] Shutting down..." << std::endl;
    m_behaviours.clear();
}


void LogicSystem::RegisterScript(const std::string& name, const BehaviourFCT& script, ScriptID id)
{
    //Adds a new script to the system
    m_behaviours[name] = script;
    m_nameToID[name] = id;
    m_idToName[id] = name;
    std::cout << "[LogicSystem] Registered script: " << name << " (ID=" << static_cast<unsigned int>(id) << ")\n";
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