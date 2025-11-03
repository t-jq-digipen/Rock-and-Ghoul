#include "MonoManager.h"
#include <iostream>
#include <filesystem>
void MonoManager::Initialize(const std::string& assemblyPath)
{
    std::cout << "[MonoManager] Initializing Mono runtime...\n";

#if _WIN32
    // Get the executable's directory
    std::filesystem::path exePath = std::filesystem::path(__argv[0]).parent_path();

    // Navigate from build/Debug up to project root, then into deps/Mono
    // exePath is build/Debug, go up twice to reach project root
    std::filesystem::path projectRoot = exePath.parent_path().parent_path();
    std::filesystem::path monoLib = projectRoot / "deps" / "Mono" / "lib";
    std::filesystem::path monoEtc = projectRoot / "deps" / "Mono" / "etc";

    // Verify the path exists
    if (!std::filesystem::exists(monoLib)) {
        std::cerr << "[MonoManager] ERROR: Mono lib path does not exist: " << monoLib << "\n";
        std::cerr << "[MonoManager] Executable directory: " << exePath << "\n";
        std::cerr << "[MonoManager] Project root: " << projectRoot << "\n";
        throw std::runtime_error("Mono libraries not found");
    }

    mono_set_dirs(monoLib.string().c_str(), monoEtc.string().c_str());
    std::cout << "[MonoManager] Mono lib path: " << monoLib << "\n";
    std::cout << "[MonoManager] Mono etc path: " << monoEtc << "\n";
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
