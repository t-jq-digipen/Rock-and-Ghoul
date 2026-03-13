/* ========================================================
 * @file        MonoSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        07 NOV 2025
 *
 * @brief
 * MonoSystem integrates the C# scripting layer into the ECS.
 *
 * Responsibilities:
 * - Initialize Mono via MonoManager and load GameScripts.dll
 * - Maintain mapping: C# script name <-> MonoScriptID
 * - Maintain per-entity MonoObject instances for each script
 * - Call Init() once and Update(float dt) every frame
 *
 * This mirrors the LogicSystem + BehaviourComponent pattern.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include "FilePath.h"

#include "System.h"
#include "MonoManager.h"
#include "Entity.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

 // ========================================================
 //                  MONO SCRIPT ID ENUM
 // ========================================================
enum class MonoScriptID : unsigned int
{
    NONE = 0,
    ROTATOR,
    MOVER,
    // add more as you need
};


// Forward declaration definition is in MonoScriptComponent.h
struct MonoScriptComponent;

// ========================================================
//                       MonoSystem
// ========================================================
class MonoSystem : public ISystem
{
public:
    // Singleton
    static MonoSystem& Get()
    {
        static MonoSystem instance;
        return instance;
    }

    MonoSystem(const MonoSystem&) = delete;
    MonoSystem& operator=(const MonoSystem&) = delete;

    // ISystem interface
    void Initialize() override;
    void Update(float dt) override;
    void Shutdown() override;

    // ----------------------------------------------------
    // Script registry API (call this during startup)
    // ----------------------------------------------------
    void RegisterScript(const std::string& className, MonoScriptID id);

    // Name <-> ID accessors (used by editor & serialization)
    const std::unordered_map<std::string, MonoScriptID>& GetNameToIDMap() const
    {
        return m_nameToID;
    }

    const std::unordered_map<MonoScriptID, std::string>& GetIDToNameMap() const
    {
        return m_idToName;
    }

    const std::string& GetScriptName(MonoScriptID id) const
    {
        static std::string empty;
        auto it = m_idToName.find(id);
        return (it != m_idToName.end()) ? it->second : empty;
    }

    MonoScriptID GetScriptID(const std::string& name) const
    {
        auto it = m_nameToID.find(name);
        return (it == m_nameToID.end()) ? MonoScriptID::NONE : it->second;
    }

    // Called by MonoScriptComponent when a script is removed
    void DestroyInstance(Entity entity, MonoScriptID id);

private:
    MonoSystem() = default;

    using InstanceMap =
        std::unordered_map<Entity,
        std::unordered_map<MonoScriptID, MonoObject*>>;

    std::filesystem::file_time_type m_lastWriteTime{};
    std::string m_dllPath = (FilePath::ASSETS_FOLDER + "Scripts/GameScripts.dll");

    // Name <-> ID
    std::unordered_map<std::string, MonoScriptID> m_nameToID;
    std::unordered_map<MonoScriptID, std::string> m_idToName;

    // Per-entity per-script instances
    InstanceMap m_instances;
};
