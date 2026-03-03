/* ========================================================
 * @file        LogicSystem.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan       | t.junquan@digipen.edu
 * @date        07 NOV 2025
 *
 * @brief
 * Manages and updates all C++ behaviour scripts registered in the ECS.
 *
 * The LogicSystem serves as the gameplay logic. It retrieves all entities
 * that contain a BehaviourComponent and executes their behaviour functions each frame.

 * Each BehaviourComponent maintains a list of script IDs
 * that correspond to registered logic routines stored in system behaviour map.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
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
    CAMERA_TRACK,
    ROCK_SCRIPT,
    MAINMENU_BG,
    SPIRIT,
    SPIRITMELEE,
    PORTAL,
    OBSTACLE,
    BASIC_PLAYER_INPUT //trying to put wasd
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


    /**
    * @brief Registers a script into the LogicSystem.
    *
    * @param name Name of the script
    * @param script BehaviourFCT of the script
    * @param id ScriptID of the script
    * 
    */
    void RegisterScript(const std::string& name, const BehaviourFCT& script, ScriptID id);

    /**
    * @brief Retrieves behaviour using script name.
    *
    * @param name Name of the script
    * @return BehaviourFCT of script name
    *
    */
    BehaviourFCT* GetBehaviour(const std::string& name);

    /**
    * @brief Retrieves behaviour using script ID.
    *
    * @param id ID of script
    * @return BehaviourFCT of script ID
    *
    */
    BehaviourFCT* GetBehaviourByID(ScriptID id);

    // Map getters
    const std::unordered_map<std::string, ScriptID>& GetNameToIDMap() const { return m_nameToID; }
    const std::unordered_map<ScriptID, std::string>& GetIDToNameMap() const { return m_idToName; }

private:
    LogicSystem() = default;  // Private constructor for singleton
    std::unordered_map<std::string, BehaviourFCT> m_behaviours; // various behaviours (contain scripts)
    std::unordered_map<std::string, ScriptID>     m_nameToID;   // name to ID map
    std::unordered_map<ScriptID, std::string>     m_idToName;   // ID to name map
};