#include "MonoManager.h"
#include <iostream>
#include <filesystem>
void MonoManager::Initialize(const std::string& assemblyPath)
{
    std::cout << "[MonoManager] Initializing Mono runtime...\n";

#if _WIN32
    mono_set_dirs("C:/Program Files/Mono/lib", "C:/Program Files/Mono/etc");
#elif __APPLE__
    mono_set_dirs("/Library/Frameworks/Mono.framework/Versions/Current/lib",
        "/Library/Frameworks/Mono.framework/Versions/Current/etc");
#else
    mono_set_dirs("/usr/lib", "/etc");
#endif

    domain = mono_jit_init_version("GameDomain", "v4.0.30319");
    if (!domain)
        throw std::runtime_error("Failed to initialize Mono domain");
    std::cout << "[MonoManager] Looking for assembly at: "
        << std::filesystem::absolute(assemblyPath) << "\n";
    assembly = mono_domain_assembly_open(domain, assemblyPath.c_str());
    if (!assembly)
        throw std::runtime_error("Failed to load assembly: " + assemblyPath);

    image = mono_assembly_get_image(assembly);
    if (!image)
        throw std::runtime_error("Failed to get Mono image from assembly");

    std::cout << "[MonoManager] Loaded C# assembly: " << assemblyPath << "\n";
}

void MonoManager::InvokeMethod(const std::string& className, const std::string& methodName, float dt)
{
    if (!image) return;

    MonoClass* klass = mono_class_from_name(image, "", className.c_str());
    if (!klass)
    {
        std::cerr << "[MonoManager] Class not found: " << className << "\n";
        return;
    }

    MonoObject* instance = mono_object_new(domain, klass);
    mono_runtime_object_init(instance);

    MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), 1);
    if (!method)
    {
        std::cerr << "[MonoManager] Method not found: " << methodName << " in " << className << "\n";
        return;
    }

    void* args[1];
    args[0] = &dt;

    mono_runtime_invoke(method, instance, args, nullptr);
}

void MonoManager::Shutdown()
{
    if (domain)
    {
        std::cout << "[MonoManager] Shutting down Mono runtime...\n";
        mono_jit_cleanup(domain);
        domain = nullptr;
    }
}
