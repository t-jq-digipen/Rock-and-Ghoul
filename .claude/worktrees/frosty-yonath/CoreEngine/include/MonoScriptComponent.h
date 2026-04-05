#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Entity.h"
#include "DataDefs.h"
#include "MonoSystem.h"


struct MonoScriptComponent
{
    Entity entityID = INVALID_ENTITY;

    // Multiple C# scripts per entity
    std::vector<MonoScriptID> scriptIDList;
    std::unordered_map<MonoScriptID, bool> startedScripts; // Init() tracking

    MonoScriptComponent() = default;

    explicit MonoScriptComponent(const std::string& name)
    {
        AddScript(name);
    }

    void FixID(Entity eID)
    {
        entityID = eID;
    }

    // --------------------------------------------------
    // Add / Remove / Check Scripts
    // --------------------------------------------------
    void AddScript(const std::string& name)
    {
        MonoSystem& mono = MonoSystem::Get();
        MonoScriptID id = mono.GetScriptID(name);

        if (id == MonoScriptID::NONE)
            return;

        if (startedScripts.count(id))
            return; // already added

        scriptIDList.push_back(id);
        startedScripts[id] = false;
    }

    bool HasScript(const std::string& name) const
    {
        MonoScriptID id = MonoSystem::Get().GetScriptID(name);
        return startedScripts.count(id) != 0;
    }

    void RemoveScript(MonoScriptID id)
    {
        if (!startedScripts.count(id))
            return;

        startedScripts.erase(id);
        std::erase(scriptIDList, id);

        MonoSystem::Get().DestroyInstance(entityID, id);
    }

    // --------------------------------------------------
    // Editor
    // --------------------------------------------------
    Editor::ComponentData EditorData() const
    {
        Editor::ComponentData data("Mono Script");
        return data; // UI is handled in ImGuiSystem
    }

    // --------------------------------------------------
    // Serialization (save Mono scripts by NAME, not ID)
    // --------------------------------------------------
    using json = nlohmann::json;

    std::string Serialize() const
    {
        json j;
        MonoSystem& mono = MonoSystem::Get();

        std::vector<std::string> names;
        names.reserve(scriptIDList.size());

        for (MonoScriptID id : scriptIDList)
        {
            names.push_back(mono.GetScriptName(id));
        }

        j["scripts"] = names;
        return j.dump();
    }

    static MonoScriptComponent DeSerialize(Entity eID, const std::string& data)
    {
        MonoScriptComponent comp;
        comp.entityID = eID;

        if (data.empty())
            return comp;

        json j = json::parse(data);
        //MonoSystem& mono = MonoSystem::Get();

        for (auto& name : j["scripts"])
        {
            comp.AddScript(name);
        }

        return comp;
    }
};
