//#pragma once
//#include "System.h"
//#include "MonoManager.h"
//#include "Registry.h"
//#include "MonoScriptComponent.h"
//#include <iostream>
//
//class MonoSystem : public ISystem
//{
//public:
//    void Initialize() override
//    {
//        std::cout << "[MonoSystem] Initializing Mono runtime...\n";
//        MonoManager::Get().Initialize("../Assets/Scripts/GameScripts.dll");
//    }
//    void Update(float dt) override
//    {
//        Registry& reg = Registry::Get();
//        auto& scripts = reg.Get_ComponentMap<MonoScriptComponent>();
//
//        for (auto& [entity, script] : scripts)
//        {
//            // If the entity doesn't have a display name yet, give it one
//            std::string currentName = reg.Get_EntityName(entity);
//            if (currentName.rfind("Entity_", 0) == 0) // starts with "Entity_"
//            {
//                reg.Set_EntityName(entity, "C# " + script.className);
//            }
//
//            if (!script.instance)
//            {
//                MonoClass* klass = mono_class_from_name(MonoManager::Get().GetImage(), "", script.className.c_str());
//                if (!klass) continue;
//
//                script.instance = mono_object_new(MonoManager::Get().GetDomain(), klass);
//                mono_runtime_object_init(script.instance);
//
//                // Assign EntityID safely
//                MonoClassField* field = mono_class_get_field_from_name(klass, "EntityID");
//                if (field)
//                {
//                    unsigned int eid = entity;
//                    mono_field_set_value(script.instance, field, &eid);
//                }
//
//                script.entityID = entity;
//            }
//
//            // call Update(dt)
//            if (script.instance)
//            {
//                MonoClass* klass = mono_object_get_class(script.instance);
//                MonoMethod* update = mono_class_get_method_from_name(klass, "Update", 1);
//                if (update)
//                {
//                    void* args[1] = { &dt };
//                    mono_runtime_invoke(update, script.instance, args, nullptr);
//                }
//            }
//        }
//    }
//
//
//
//    void Shutdown() override
//    {
//        MonoManager::Get().Shutdown();
//    }
//};
