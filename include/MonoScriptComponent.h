#pragma once
#include <string>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include "Entity.h"
#include "DataDefs.h"

struct MonoScriptComponent
{
    std::string className;
    MonoObject* instance = nullptr;
    Entity entityID = INVALID_ENTITY; // link back to ECS entity

    // === Editor support ===
    Editor::ComponentData EditorData() const
    {
        Editor::ComponentData data("Mono Script");
        data.Add_Data("Class", Editor::Widget_String{ nullptr, className });
        return data;
    }

    // === Serialization ===
    std::string Serialize() const
    {
        return className; // simplest possible serialization
    }

    static MonoScriptComponent DeSerialize(const std::string& data)
    {
        MonoScriptComponent comp;
        comp.className = data; // reconstruct from saved string
        return comp;
    }
};
