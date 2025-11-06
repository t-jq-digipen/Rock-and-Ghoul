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
    }

    void Update(float dt) override
    {
        //SHOULD IMPLEMENT HOT RELOADING HERE

        Registry& reg = Registry::Get();
        auto& scripts = reg.Get_ComponentMap<MonoScriptComponent>();

        for (auto& [entity, script] : scripts)
        {
            //C# entities readable names
            std::string currentName = reg.Get_EntityName(entity);
            //reverse find
            if (currentName.rfind("Entity_", 0) == 0)
                reg.Set_EntityName(entity, "C# " + script.className);

            if (!script.instance)
            {

                //void CallPrint()
                //{
                //    // Get class info
                //    MonoImage* image = mono_assembly_get_image(assembly);
                //    MonoClass* _class = mono_class_from_name(image, _namespace, _name);
                //    if (!_class) return;

                //    // Get method info
                //    // Last parameter is # of parameters in method definition
                //    MonoMethod* method = mono_class_get_method_from_name(_class, "Print", 0);
                //    if (!method) return;

                //    // Call method
                //    MonoObject* exception = nullptr; // Exception gets stored in this object
                //    mono_runtime_invoke(method, nullptr, nullptr, &exception);
                //    if (exception)
                //        mono_print_unhandled_exception(exception);
                //}
                MonoClass* constructorMethod = mono_class_from_name(MonoManager::Get().GetImage(), "", script.className.c_str());
                if (!constructorMethod)
                {
                    std::cerr << "[MonoSystem] Class not found: " << script.className << "\n";
                    continue;
                }

                script.instance = mono_object_new(MonoManager::Get().GetDomain(), constructorMethod);
                mono_runtime_object_init(script.instance);

                // Assign EntityID field in C#
                MonoClassField* field = mono_class_get_field_from_name(constructorMethod, "EntityID");
                if (field)
                {
                    unsigned int eid = entity;
                    mono_field_set_value(script.instance, field, &eid);
                }

                script.entityID = entity;
            }

            // call C# Update(float dt)
            //https://www.giorgosdimtsas.net/blog/embedding-mono-in-a-c-application/
            if (script.instance)
            {
                MonoClass* MonoClass = mono_object_get_class(script.instance);
                if (!MonoClass)
                {
                    std::cerr << "[MonoSystem] Failed to get MonoClass for instance of " << script.className << "\n";
                    continue;
                }

                // find the GetName method
                MonoMethod* getNameMethod = mono_class_get_method_from_name(MonoClass, "Update", 1);
                if (!getNameMethod)
                {
                    std::cerr << "[MonoSystem] C# class '" << script.className << "' does not have an Update(float) method.\n";
                    continue;
                }

                //argument array
                void* args[1];
                args[0] = &dt;

                // finally, invoke the constructor
                MonoObject* exception = nullptr;
                mono_runtime_invoke(getNameMethod, script.instance, args, &exception);
                if (exception)
                {
                    std::cerr << "[MonoSystem] Exception in " << script.className << ".Update():\n";
                    mono_print_unhandled_exception(exception);
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
