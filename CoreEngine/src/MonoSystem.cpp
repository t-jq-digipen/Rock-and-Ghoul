/* ========================================================
 * @file        MonoSystem.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 100%
 * @date        07 NOV 2025
 *
 * @brief
 * Implements MonoSystem, which manages all C# scripts using
 * Mono and integrates them into the ECS update loop.
 *
 * - Keeps name <-> MonoScriptID mappings
 * - Creates MonoObject instances per (entity, scriptID)
 * - Calls Init() once and Update(float dt) every frame
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "MonoSystem.h"
#include "MonoScriptComponent.h"
#include "Registry.h"
#include <iostream>

void MonoSystem::Initialize()
{
    std::cout << "[MonoSystem] Initializing Mono runtime...\n";
    MonoManager::Get().Initialize(m_dllPath);
}

void MonoSystem::Update(float dt)
{
    Registry& reg = Registry::Get();
    auto& scripts = reg.Get_ComponentMap<MonoScriptComponent>();

    for (auto& [entity, comp] : scripts)
    {
        // give a nicer name if still "Entity_x"
        if (!comp.scriptIDList.empty())
        {
            std::string currentName = reg.Get_EntityName(entity);
            if (currentName.rfind("Entity_", 0) == 0)
            {
                MonoScriptID firstID = comp.scriptIDList.front();
                const std::string& cname = GetScriptName(firstID);
                if (!cname.empty())
                    reg.Get_EntityNameMap()[entity] = "C# " + cname;
            }
        }

        for (MonoScriptID sID : comp.scriptIDList)
        {
            if (sID == MonoScriptID::NONE)
                continue;

            // resolve class name
            const std::string& className = GetScriptName(sID);
            if (className.empty())
            {
                std::cerr << "[MonoSystem] Unknown MonoScriptID on entity " << entity << "\n";
                continue;
            }

            MonoImage* image = MonoManager::Get().GetImage();
            MonoClass* klass = mono_class_from_name(image, "", className.c_str());
            if (!klass)
            {
                std::cerr << "[MonoSystem] Class not found: " << className << "\n";
                continue;
            }

            // get/create instance
            MonoObject*& instanceRef = m_instances[entity][sID];

            if (!instanceRef)
            {
                instanceRef = mono_object_new(MonoManager::Get().GetDomain(), klass);
                mono_runtime_object_init(instanceRef);

                // Set "public uint EntityID" if available
                MonoClassField* field =
                    mono_class_get_field_from_name(klass, "EntityID");
                if (field)
                {
                    unsigned int eid = entity;
                    mono_field_set_value(instanceRef, field, &eid);
                }
            }

            // Init() once
            if (!comp.startedScripts[sID])
            {
                MonoMethod* initMethod =
                    mono_class_get_method_from_name(klass, "Init", 0);

                if (initMethod)
                {
                    MonoObject* exception = nullptr;
                    mono_runtime_invoke(initMethod, instanceRef, nullptr, &exception);
                    if (exception)
                    {
                        std::cerr << "[MonoSystem] Exception in "
                            << className << ".Init():\n";
                        mono_print_unhandled_exception(exception);
                    }
                }

                comp.startedScripts[sID] = true;
            }

            // Update(float dt) every frame
            MonoMethod* updateMethod =
                mono_class_get_method_from_name(klass, "Update", 1);

            if (!updateMethod)
            {
                std::cerr << "[MonoSystem] C# class '" << className
                    << "' does not have an Update(float) method.\n";
                continue;
            }

            void* args[1] = { &dt };
            MonoObject* exception = nullptr;

            mono_runtime_invoke(updateMethod, instanceRef, args, &exception);
            if (exception)
            {
                std::cerr << "[MonoSystem] Exception in "
                    << className << ".Update():\n";
                mono_print_unhandled_exception(exception);
            }
        }
    }
}

void MonoSystem::Shutdown()
{
    std::cout << "[MonoSystem] Shutting down Mono runtime...\n";
    m_instances.clear();
    MonoManager::Get().Shutdown();
}

void MonoSystem::RegisterScript(const std::string& className, MonoScriptID id)
{
    m_nameToID[className] = id;
    m_idToName[id] = className;

    std::cout << "[MonoSystem] Registered C# script: "
        << className << " (ID=" << static_cast<unsigned int>(id) << ")\n";
}

void MonoSystem::DestroyInstance(Entity entity, MonoScriptID id)
{
    auto entIt = m_instances.find(entity);
    if (entIt == m_instances.end())
        return;

    auto& inner = entIt->second;
    auto it = inner.find(id);
    if (it != inner.end())
        inner.erase(it);

    if (inner.empty())
        m_instances.erase(entIt);
}
