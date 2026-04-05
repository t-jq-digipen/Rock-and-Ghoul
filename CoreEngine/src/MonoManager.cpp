/* ========================================================
 * @file        MonoManager.cpp
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 100%
 * @date        07 NOV 2025
 *
 * @brief
 * Implements the MonoManager class, which embeds the Mono
 * runtime within the engine to enable execution of managed
 * C# scripts (.dll assembly).
 *
 * The MonoManager initialize and configure the Mono environment, load C# assembly,
 * invoke script methods, and shutdown the runtime.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ======================================================== 
 */


#include "MonoManager.h"
#include <iostream>
#include <filesystem>
void MonoManager::Initialize(const std::string& assemblyPath)
{
    std::cout << "[MonoManager] Initializing Mono\n";


    // Get the executable directory
    //C:\blabla\RockAndGhoul\build\Debug\RockAndGhoul.exe C:\blabla\RockAndGhoul\build\Debug

    std::filesystem::path exePath = std::filesystem::absolute(__argv[0]).parent_path();
    std::filesystem::path projectRoot = exePath;
    // Navigate from build/Debug up to project root, then into deps/Mono
    // exePath is build/Debug, go up twice to reach project root
    while (!std::filesystem::exists(projectRoot / "deps") && projectRoot.has_parent_path())
        projectRoot = projectRoot.parent_path();

    if (!std::filesystem::exists(projectRoot / "deps" / "Mono" / "lib")) {
        std::cerr << "[MonoManager] Could not locate Mono deps folder relative to " << projectRoot << "\n";
        throw std::runtime_error("Mono libraries not found");
    }
   // std::filesystem::path exePath = std::filesystem::path(__argv[0]).parent_path();

    // Navigate from build/Debug up to project root, then into deps/Mono
    // exePath is build/Debug, go up twice to reach project root
    //std::filesystem::path projectRoot = exePath.parent_path().parent_path();
    std::filesystem::path monoLib = projectRoot / "deps" / "Mono" / "lib";
    std::filesystem::path monoEtc = projectRoot / "deps" / "Mono" / "etc";

    // Verify the path exists
    if (!std::filesystem::exists(monoLib)) {
        std::cerr << "[MonoManager] Mono lib path does not exist: " << monoLib << "\n";
        std::cerr << "[MonoManager] Executable directory: " << exePath << "\n";
        std::cerr << "[MonoManager] Project root: " << projectRoot << "\n";
        throw std::runtime_error("Mono libraries not found");
    }
    //https://www.giorgosdimtsas.net/blog/embedding-mono-in-a-c-application/
    mono_set_dirs(monoLib.string().c_str(), monoEtc.string().c_str());
    std::cout << "[MonoManager] Mono lib path: " << monoLib << "\n";
    std::cout << "[MonoManager] Mono etc path: " << monoEtc << "\n";

    domain = mono_jit_init_version("GameEngineDomain", "v4.0.30319");
    if (!domain)
        throw std::runtime_error("Failed to initialize Mono domain");

    std::cout << "[MonoManager] Mono domain initialized successfully.\n";

    assembly = mono_domain_assembly_open(domain, assemblyPath.c_str());
    if (!assembly) {
        std::cerr << "[MonoManager] Failed to load assembly: " << assemblyPath << "\n";
        return;
    }
    std::cout << "[MonoManager] Assembly loaded OK: " << assemblyPath << "\n";

    image = mono_assembly_get_image(assembly);
    if (!image)
        throw std::runtime_error("Failed to get Mono image from assembly");

    std::cout << "[MonoManager] Loaded C# assembly: " << assemblyPath << "\n";
}

void MonoManager::InvokeMethod(const std::string& className, const std::string& methodName, float dt)
{
    if (!image) return;

    MonoClass* constructorMethod = mono_class_from_name(image, "", className.c_str());
    if (!constructorMethod)
    {
        std::cerr << "[MonoManager] Class not found: " << className << "\n";
        return;
    }

    MonoObject* instance = mono_object_new(domain, constructorMethod);
    mono_runtime_object_init(instance);

    MonoMethod* method = mono_class_get_method_from_name(constructorMethod, methodName.c_str(), 1);
    if (!method)
    {
        std::cerr << "[MonoManager] Method not found: " << methodName << " in " << className << "\n";
        return;
    }

    void* args[1];
    args[0] = &dt;
    // invoke the method
    // if invoking static methods, then the second argument must be NULL
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
