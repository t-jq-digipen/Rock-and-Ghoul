#pragma once
#include "BehaviourFCT.h"
#include "DataDefs.h"
#include <string>

struct BehaviourComponent
{
    std::string scriptName;  // instead of BehaviourFCT*
    bool started = false;

    Editor::ComponentData EditorData()
    {
        Editor::ComponentData data("Behaviour");
        // No editable fields in editor for now
        return data;
    }

    std::string Serialize() const
    {
        // Behaviour not serialized for now
        return "{}";
    }

    static BehaviourComponent DeSerialize(Entity, const std::string&)
    {
        return BehaviourComponent{};
    }
};