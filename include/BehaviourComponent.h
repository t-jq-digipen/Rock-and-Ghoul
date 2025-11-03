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
    BehaviourComponent() = default;
    BehaviourComponent(const std::string& name)
    {
        SetScript(name);
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
    void SetScript(const std::string& name)
    {
        scriptName = name;
        LogicSystem& logicSys = LogicSystem::Get();
        auto& map = logicSys.GetNameToIDMap();

        if (auto it = map.find(name); it != map.end())
            scriptID = it->second;
        else
            scriptID = ScriptID::NONE;
    }
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
        j["scriptName"] = scriptName;
        j["scriptID"] = static_cast<unsigned int>(scriptID);
        return j.dump();
        //return "{}";
    }

    static BehaviourComponent DeSerialize(Entity, const std::string& data)
    {
        BehaviourComponent bc;
        auto j = nlohmann::json::parse(data);
        bc.scriptName = j.value("scriptNames", "");
        bc.scriptID = static_cast<ScriptID>(j.value("scriptID", 0u));
        bc.started = false;

        bc.SetScript(bc.scriptName);
        return bc;
    }
};