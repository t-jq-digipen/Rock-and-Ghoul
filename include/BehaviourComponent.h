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
    ScriptID scriptID = ScriptID::NONE;        // Internal enum for fast lookup
    std::string scriptName = "";               // Human-readable name
    bool started = false;                      // Whether Init() ran yet

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
    Editor::ComponentData EditorData()
    {
        Editor::ComponentData data("Behaviour");
        Editor::Widget_Dropdown dropdown;

        LogicSystem& logicSys = LogicSystem::Get();
        for (auto& [name, id] : logicSys.GetNameToIDMap())
            dropdown.Add_Option(name, static_cast<unsigned int>(id));

        dropdown.ref = reinterpret_cast<unsigned int*>(&scriptID);
        data.Add_Data("Script", dropdown);
        return data;
    }

    std::string Serialize() const
    {
        nlohmann::json j;
        j["scriptNames"] = scriptName;
        j["scriptIDs"] = static_cast<unsigned int>(scriptID);
        return j.dump();
        //return "{}";
    }

    static BehaviourComponent DeSerialize(Entity, const std::string& data)
    {
        BehaviourComponent bc;
        auto j = nlohmann::json::parse(data);
        bc.scriptName = j.value("scriptName", "");
        bc.scriptID = static_cast<ScriptID>(j.value("scriptID", 0u));
        bc.started = false;
        return bc;
    }
};