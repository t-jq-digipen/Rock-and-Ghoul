#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "Entity.h"
#include "BehaviourFCT.h"
#include "DataDefs.h"
#include "LogicSystem.h"
struct BehaviourComponent
{
    //std::string scriptName;  // instead of BehaviourFCT*
    //bool started = false;
    //std::vector<ScriptID> scriptIDs;
    //std::vector<std::string> scriptNames; // multiple scripts
    //std::vector<bool> startedStates;

    std::unordered_map<ScriptID, bool> startedScripts;
    std::vector<ScriptID> scriptIDList;

    // ScriptID scriptID = ScriptID::NONE;        // Internal enum for fast lookup
    // std::string scriptName = "";               // Human-readable name
    // bool started = false;                      // Whether Init() ran yet
    BehaviourComponent() = default;
    ~BehaviourComponent()
    {
        for (ScriptID scriptID : scriptIDList)
        {
            if (scriptID != ScriptID::NONE)
            {
                LogicSystem& logicSys = LogicSystem::Get();
                BehaviourFCT* script = logicSys.GetBehaviourByID(scriptID);
                if (script && script->End)
                    script->End(0);
            }
        }

        startedScripts.clear();
        scriptIDList.clear();
        
    }

    BehaviourComponent(const std::string& name)
    {
        AddScript(name);
    }
    //Editor::ComponentData EditorData() {
    //    Editor::ComponentData data("Behaviour");
    //    Editor::Widget_Dropdown dropdown;
    //    static LogicSystem& logicSys = LogicSystem::Get();
    //    for (auto& [name, id] : logicSys.GetNameToIDMap())
    //        dropdown.Add_Option(name, static_cast<unsigned int>(id));

    //    if (!scriptIDs.empty())
    //        dropdown.ref = reinterpret_cast<unsigned int*>(&scriptIDs[0]);

    //    data.Add_Data("Scripts", dropdown);
    //    return data;
    //}
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


            if (sID != ScriptID::NONE)
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