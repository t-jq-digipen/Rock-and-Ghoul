#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "Entity.h"
#include "BehaviourFCT.h"
#include "DataDefs.h"
#include "LogicSystem.h"


//FILLTHISHITJQ
struct BehaviourComponent
{
    Entity entityID{};

    std::unordered_map<ScriptID, bool> startedScripts;
    std::vector<ScriptID> scriptIDList;

    // ScriptID scriptID = ScriptID::NONE;        // Internal enum for fast lookup
    std::string scriptName = "";               // Human-readable name
    bool started = false;                      // Whether Init() ran yet
    BehaviourComponent() = default;
    ~BehaviourComponent()
    {
        for (ScriptID scriptID : scriptIDList)
        {
            if (scriptID != ScriptID::NONE)
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

    BehaviourComponent(const std::string& name)
    {
        AddScript(name);
    }

    void FixID(Entity eID)
    {
        entityID = eID;
    }

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

    void RemoveScript(const std::string& name)
    {
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetNameToIDMap();

        if (auto it = map.find(name); it != map.end())
        {
            ScriptID sID = it->second;
            if (startedScripts.find(sID) == startedScripts.end()) { return; } // Script not added


            if (startedScripts[sID] &&  sID != ScriptID::NONE)
            {
                BehaviourFCT* script = logicSys.GetBehaviourByID(sID);
                if (script && script->End)
                    script->End(0);
            }


            startedScripts.erase(sID);

            std::erase(scriptIDList, sID);
            // scriptIDList.push_back(it->second);
            // startedScripts[it->second] = false;
        }
    }

    void RemoveScript(const ScriptID& sID)
    {
        if (startedScripts.find(sID) == startedScripts.end()) { return; } // Script not added


        if (sID != ScriptID::NONE)
        {
            if (startedScripts.find(sID) == startedScripts.end()) { return; } // Script not added
            if (startedScripts[sID]) 
            {
                BehaviourFCT* script = LogicSystem::Get().GetBehaviourByID(sID);
                if (script && script->End)
                    script->End(entityID);
            }
        }


        startedScripts.erase(sID);

        std::erase(scriptIDList, sID);
    }

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

    std::string Serialize() const
    {
        json j;
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetIDToNameMap();
        std::vector<std::string> scriptNames;
        for (const ScriptID sID : scriptIDList)
        {
            scriptNames.push_back(map.find(sID)->second);
        }

        j["scripts"] = scriptNames;
        return j.dump();
        //return "{}";
    }

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