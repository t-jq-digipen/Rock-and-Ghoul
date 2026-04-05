/* ========================================================
 * @file        MonoManager.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu 100%
 * @date        07 NOV 2025
 *
 * @brief
 * Declares the MonoManager class, which embeds the Mono runtime
 * to allow execution of managed C# assemblies within the engine.
 *
 * - Initializing and configuring Mono runtime directories
 * - Loading C# assembly (GameScripts.dll)
 * - Creating Mono domain
 *
 * This system enables scripting support for game, 
 * allowing gameplay logic to be implemented
 * in C# while using the C++ ECS.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ======================================================== 
 */

#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <string>
#include <stdexcept>

 /**
  * @class MonoManager
  * @brief Singleton wrapper that manages the embedded Mono runtime.
  *
  * Provides functionality for initializing, loading, invoking, and
  * shutting down the Mono environment. The manager exposes interfaces
  * for native C++ systems to communicate with C# script classes
  * through Mono’s embedding API.
  */
class MonoManager
{
public:
    /**
    * @brief Retrieves the global MonoManager instance.
    * @return Reference to the singleton instance.
    */
    static MonoManager& Get()
    {
        static MonoManager instance;
        return instance;
    }
    /**
    * @brief Initializes the Mono runtime and loads the specified assembly.
    *
    * Automatically resolves Mono directories relative to the executable, creates a new JIT domain,
    *  and loads the target C# assembly (GameScripts.dll).
    *
    * assemblyPath Absolute or relative path to the C# assembly.
    *
    */
    void Initialize(const std::string& assemblyPath);

    /**
     * @brief Invokes a method from a managed C# class.
     *
     * Instance of the specified class, looks up
     * the target method by name, and executes it. Call
     * Update(float dt) in game script.
     *
     * className  Name of the C# class within the loaded assembly.
     * methodName Name of the method to invoke.
     * dt         value passed as argument to the method.
     *
     */
    void InvokeMethod(const std::string& className, const std::string& methodName, float dt);

    /**
     * @brief Shuts down the Mono runtime and destroys domain.
     *
     * Cleans up all managed objects and releases resources
     * allocated by the Mono JIT runtime.
     */
    void Shutdown();

    /**
   * @brief Accessor for the current Mono domain.
   * @return Pointer to the active MonoDomain.
   */
    MonoDomain* GetDomain() { return domain; }

    /**
     * @brief Accessor for the loaded Mono image.
     * @return Pointer to the active MonoImage.
     */
    MonoImage* GetImage() { return image; }
private:
    MonoManager() = default;
    ~MonoManager() = default;

    MonoDomain* domain = nullptr;
    MonoAssembly* assembly = nullptr;
    MonoImage* image = nullptr;
};
