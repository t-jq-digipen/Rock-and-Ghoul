#pragma once
#include <string>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

struct MonoScriptComponent
{
    std::string className;
    MonoObject* instance = nullptr;
    Entity entityID = INVALID_ENTITY; // link back to ECS entity
};
