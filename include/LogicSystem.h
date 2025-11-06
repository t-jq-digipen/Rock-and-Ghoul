/* ========================================================
 * @file        LogicSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu FILLTHISSHITJQ
 * @date        07 NOV 2025
 *
 * @brief
 * Manages and updates all C++ behaviour scripts registered in the ECS.
 *
 * The LogicSystem serves as the gameplaY logic. It retrieves all entities
 * that contain a BehaviourComponent and executes their behaviour functions each frame.

 * Each BehaviourComponent maintains a list of script IDs
 * that correspond to registered logic routines stored in system’s behaviour map.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <string>
#include <unordered_map>
#include "BehaviourFCT.h"
#include "System.h"

enum class ScriptID : unsigned int {
    NONE = 0,
    GHOUL,
    PATROL_ENEMY,
    PLAYER,
    CAMERA_TRACK
};

/**
 * @class LogicSystem
 * @brief ECS system responsible for C++ behaviour scripts.
 *
 * Loop over all entities that contain a BehaviourComponent and runs
 * behaviour functions registered in the engine.
 *
 * Each BehaviourComponent have a list of ScriptIDs that map to
 * BehaviourFCT (function pointers) stored in the LogicSystem.
 *
 * For every active script, the system automatically calls:
 * - Init(Entity) once on the first update
 * - Update(Entity, float dt) every subsequent frame
 *
 * The system skips updates while the game is paused or in edit
 * mode.
 *
 */
class LogicSystem : public ISystem
{
public:
    // Singleton pattern
    static LogicSystem& Get() {
        static LogicSystem instance;
        return instance;
    }

    // Delete copy constructor and assignment
    LogicSystem(const LogicSystem&) = delete;
    LogicSystem& operator=(const LogicSystem&) = delete;

    /**
      * @brief Initializes the LogicSystem.
      *
      * Called once at engine startup to prepare internal maps
      * for behaviour registration and log initialization.
     */
    void Initialize() override;

    /**
     * @brief Updates all registered behaviour scripts.
     *
     * For every entity that contains a BehaviourComponent,
     * retrieves each registered script and executes its
     * Init() (if not yet started) and Update(float dt)
     * callbacks.
     *
     * @param dt Delta time since the last frame.
     *
     */
    void Update(float dt) override;

    /**
    * @brief Shuts down the LogicSystem.
    *
    * Clears all registered behaviours and releases any
    * stored script mappings.
    */
    void Shutdown() override;

    //FILLTHISSHITJQ
    void RegisterScript(const std::string& name, const BehaviourFCT& script, ScriptID id);
    BehaviourFCT* GetBehaviour(const std::string& name);
    BehaviourFCT* GetBehaviourByID(ScriptID id);

    const std::unordered_map<std::string, ScriptID>& GetNameToIDMap() const { return m_nameToID; }
    const std::unordered_map<ScriptID, std::string>& GetIDToNameMap() const { return m_idToName; }

private:
    LogicSystem() = default;  // Private constructor for singleton
    std::unordered_map<std::string, BehaviourFCT> m_behaviours; //various behaviours (contain scripts)
    std::unordered_map<std::string, ScriptID>     m_nameToID;
    std::unordered_map<ScriptID, std::string>     m_idToName;
};