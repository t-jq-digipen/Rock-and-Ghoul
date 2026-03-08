/* ========================================================
 * @file	    BehaviourComponent.h
 * @author	    Tan Jun Quan | t.junquan@digipen.edu
 * @date	    07 NOV 2025
 *
 * @brief	Behaviour Component
 *
 * This file declares and defines the Behaviour Component,
 * which is used to hold C++ scripting logic! It holds a
 * list of behaviour scripts which can be attached to entities.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "Entity.h"
#include "BehaviourFCT.h"
#include "DataDefs.h"
#include "LogicSystem.h"


 /*!
  * @struct BehaviourComponent
  * @brief Component used to attach and manage behaviour scripts on an entity.
  *
  * Stores a list of script IDs, initialization state for each script, and
  * provides operations to add, remove, serialize, and deserialize scripts.
  *
  * The component ensures script `End()` callbacks run when removed or when
  * the component itself is destroyed.
  */
struct BehaviourComponent
{
    Entity entityID{};                                  // ID of entity it is attached to, must store to update the components through registry

    std::unordered_map<unsigned int, bool> startedScripts;  // Map of script ID to bool to check if a script has started
    std::vector<unsigned int> scriptIDList;                 // List of script IDs attached to the entity

    std::string scriptName = "";                        // Human-readable name
    bool started = false;                               // Whether Init() ran yet

    /**
     * @brief Default constructor.
     */
    BehaviourComponent() = default;

    /**
     * @brief Destructor. Calls End() on all started scripts.
     */
    ~BehaviourComponent()
    {
        for (unsigned int scriptID : scriptIDList)
        {
            if (startedScripts.contains(scriptID))
            {
                if (startedScripts[scriptID])
                {
                    LogicSystem& logicSys = LogicSystem::Get();
                    BehaviourFCT* script = logicSys.GetBehaviourByID(scriptID);
                    if (script && script->End)
                        script->End(entityID);
                }
            }
        }

        startedScripts.clear();
        scriptIDList.clear();
        
    }

    /**
     * @brief Construct a BehaviourComponent with one script by name.
     * @param name Name of the script to attach.
     */
    BehaviourComponent(const std::string& name)
    {
        AddScript(name);
    }

    /**
     * @brief Updates the stored entity ID.
     * @param eID New entity ID.
     */
    void FixID(Entity eID)
    {
        entityID = eID;
    }

    /**
     * @brief Adds a behaviour script by name.
     *
     * Looks up the ScriptID using LogicSystem. If already added,
     * the function does nothing.
     *
     * @param name Name of the script to attach.
     */
    void AddScript(const std::string& name)
    {
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetNameToIDMap();

        if (auto it = map.find(name); it != map.end())
        {
            if (startedScripts.find(it->second) != startedScripts.end()) { return; } // Script alr added

            scriptIDList.push_back(it->second);
            startedScripts[it->second] = false;
        }
    }

    /**
     * @brief Removes a script by name.
     *
     * If the script was started, its End() function is called before removal.
     *
     * @param name Name of the script to remove.
     */
    void RemoveScript(const std::string& name)
    {
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetNameToIDMap();

        if (auto it = map.find(name); it != map.end())
        {
            unsigned int sID = it->second;
            if (startedScripts.find(sID) == startedScripts.end()) { return; } // Script not added


            if (startedScripts[sID])
            {
                BehaviourFCT* script = logicSys.GetBehaviourByID(sID);
                if (script && script->End)
                    script->End(entityID);
            }


            startedScripts.erase(sID);

            std::erase(scriptIDList, sID);
            // scriptIDList.push_back(it->second);
            // startedScripts[it->second] = false;
        }
    }

    /**
     * @brief Removes a script by ScriptID.
     * 
     * If the script was started, its End() function is called before removal.
     * 
     * @param sID Script ID to remove.
     */
    void RemoveScript(const unsigned int& sID)
    {
        if (startedScripts.find(sID) == startedScripts.end()) { return; } // Script not added

        if (startedScripts[sID]) 
        {
            BehaviourFCT* script = LogicSystem::Get().GetBehaviourByID(sID);
            if (script && script->End)
                script->End(entityID);
        }

        startedScripts.erase(sID);

        std::erase(scriptIDList, sID);
    }

    /**
     * @brief Checks whether the component has a script by name.
     * @param name Script name.
     * @return True if the script is attached, false otherwise.
     *
     * @throws std::runtime_error If the script name does not exist in LogicSystem.
     */
    bool HasScript(const std::string& name)
    {
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetNameToIDMap();

        auto it = map.find(name);
        auto end = map.end();
        if (it != map.end())
        {
            if (startedScripts.find(it->second) == startedScripts.end())
            {
                return false;
            }
            return true;
        }
        throw std::runtime_error("has a script that dun exist?\n");
        return false;
        

    }

    /**
     * @brief Editor integration for BehaviourComponent.
     * 
     * This data field is empty as it is defined in ImGuiSystem.cpp.
     * 
     * @return Empty ComponentData only holding Component Name.
     */
    Editor::ComponentData EditorData()
    {
        Editor::ComponentData data("Behaviour");
        /*Editor::Widget_Dropdown dropdown;

        LogicSystem& logicSys = LogicSystem::Get();
        for (auto& [name, id] : logicSys.GetNameToIDMap())
            dropdown.Add_Option(name, static_cast<unsigned int>(id));

        dropdown.ref = reinterpret_cast<unsigned int*>(&scriptID);
        data.Add_Data("Script", dropdown);*/
        return data;
    }

    using json = nlohmann::json;

    /**
     * @brief Serializes attached scripts into JSON.
     * @return A JSON-formatted string containing script names.
     */
    std::string Serialize() const
    {
        json j;
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetIDToNameMap();
        std::vector<std::string> scriptNames;
        for (const unsigned int sID : scriptIDList)
        {
            scriptNames.push_back(map.find(sID)->second);
        }

        j["scripts"] = scriptNames;
        return j.dump();
        //return "{}";
    }

    /**
     * @brief Creates a BehaviourComponent from serialized JSON.
     * @param eID Entity ID.
     * @param data JSON string containing script names.
     * @return A reconstructed BehaviourComponent.
     */
    static BehaviourComponent DeSerialize(Entity, const std::string& data)
    {
        BehaviourComponent bc;
        auto j = nlohmann::json::parse(data);

        std::vector<std::string> scriptNames = j["scripts"].get<std::vector<std::string>>();

        for (std::string script : scriptNames)
        {
            bc.AddScript(script);
        }

        return bc;
    }
};