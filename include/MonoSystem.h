#pragma once
#include "System.h"
#include "MonoManager.h"
#include "Registry.h"
#include "MonoScriptComponent.h"
#include <iostream>
#include <filesystem>

class MonoSystem : public ISystem
{
private:
    std::filesystem::file_time_type lastWriteTime{};
    std::string dllPath = "../Assets/Scripts/GameScripts.dll";

public:
    void Initialize() override
    {
        std::cout << "[MonoSystem] Initializing Mono runtime...\n";

        MonoManager::Get().Initialize(dllPath);

        if (std::filesystem::exists(dllPath))
            lastWriteTime = std::filesystem::last_write_time(dllPath);
        else
            std::cerr << "[MonoSystem] WARNING: GameScripts.dll not found at " << dllPath << "\n";
    }

    void Update(float dt) override
    {
        // --- Hot Reload Check ---
        if (std::filesystem::exists(dllPath))
        {
            auto currentWriteTime = std::filesystem::last_write_time(dllPath);

            if (currentWriteTime != lastWriteTime)
            {
                std::cout << "[MonoSystem] Detected GameScripts.dll change — reloading...\n";

                // Shutdown old Mono domain
                MonoManager::Get().Shutdown();

                // Restart Mono runtime + reload assembly
                MonoManager::Get().Initialize(dllPath);

                lastWriteTime = currentWriteTime;

                // Reset script instances to rebind to new assembly
                auto& scripts = Registry::Get().Get_ComponentMap<MonoScriptComponent>();
                for (auto& [entity, script] : scripts)
                {
                    script.instance = nullptr;
                }
            }
        }

        // --- Normal script update ---
        Registry& reg = Registry::Get();
        auto& scripts = reg.Get_ComponentMap<MonoScriptComponent>();

        for (auto& [entity, script] : scripts)
        {
            // Give C# entities readable names
            std::string currentName = reg.Get_EntityName(entity);
            if (currentName.rfind("Entity_", 0) == 0)
                reg.Set_EntityName(entity, "C# " + script.className);

            if (!script.instance)
            {
                MonoClass* klass = mono_class_from_name(MonoManager::Get().GetImage(), "", script.className.c_str());
                if (!klass)
                {
                    std::cerr << "[MonoSystem] Class not found: " << script.className << "\n";
                    continue;
                }

                script.instance = mono_object_new(MonoManager::Get().GetDomain(), klass);
                mono_runtime_object_init(script.instance);

                // Assign EntityID field in C#
                MonoClassField* field = mono_class_get_field_from_name(klass, "EntityID");
                if (field)
                {
                    unsigned int eid = entity;
                    mono_field_set_value(script.instance, field, &eid);
                }

                script.entityID = entity;
            }

            // Call C# Update(float dt)
            if (script.instance)
            {
                MonoClass* klass = mono_object_get_class(script.instance);
                MonoMethod* update = mono_class_get_method_from_name(klass, "Update", 1);
                if (update)
                {
                    void* args[1] = { &dt };
                    mono_runtime_invoke(update, script.instance, args, nullptr);
                }
            }
        }
    }

    void Shutdown() override
    {
        std::cout << "[MonoSystem] Shutting down Mono runtime...\n";
        MonoManager::Get().Shutdown();
    }
};
