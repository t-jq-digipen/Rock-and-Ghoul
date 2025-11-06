/* ========================================================
 * @function    GameScripts.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        07 NOV 2025
 *
 * @brief
 * Registers all native C++ gameplay scripts into the LogicSystem.
 *
 * This function binds each game behaviour to a BehaviourFCT structure containing
 * Init, Update, and End callbacks. Each script is then assigned a
 * unique ScriptID and registered into the LogicSystem behaviour map.

 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ======================================================== */


#pragma once
#include "LogicSystem.h"
#include "Component.h"
#include "Registry.h"
#include <iostream>

// ============================================
// GHOUL SCRIPT
// ============================================
namespace GhoulScript
{
    /**
     * @brief Called when the GhoulScript is first initialized.
     *
     * Prints a message confirming the script has started for the given entity.
     */
    //random entity name lol
    inline void Start(Entity gob)
    {
        std::cout << "Ghoul script started for entity " << gob << std::endl;
    }

    /**
     * @brief Rotates the entity at 45 degrees per second.
     *
     * Retrieves the TransformComponent and updates its rotation value
     * each frame. Keeps the rotation angle clamped between 0–360 degrees.
     *
     */
    inline void Update(Entity gob, float dt)
    {
        Registry& registry = Registry::Get();

        if (registry.Has_Component<TransformComponent>(gob))
        {
            auto& transform = registry.Get_Component<TransformComponent>(gob);

            // rot 45 degrees per second
            transform.rot += 45.0f * dt;


            if (transform.rot >= 360.0f) {
                transform.rot -= 360.0f;
            }
        }
    }
    /**
     * @brief Called when the GhoulScript is stopped or removed.
     *
     * Logs a message to indicate that the script has ended for this entity.
     */
    inline void End(Entity gob)
    {
        std::cout << "Ghoul script ended for entity " << gob << std::endl;
    }
}
// ============================================
// PATROL ENEMY SCRIPT
// ============================================
namespace PatrolEnemyScript
{
    /**
     * @brief Initializes the PatrolEnemyScript.
     *
     * Logs the starting position and sets up patrol data used in Update().
     *
     */
    inline void Start(Entity e)
    {
        std::cout << "[PatrolEnemy] Starting patrol for entity " << e << std::endl;

        //initial position as patrol start point
        Registry& reg = Registry::Get();
        if (reg.Has_Component<TransformComponent>(e)) {
            auto& transform = reg.Get_Component<TransformComponent>(e);
            std::cout << "[PatrolEnemy] Starting position: (" << transform.pos.x << ", " << transform.pos.y << ")" << std::endl;
        }
    }
    /**
     * @brief Moves the entity back and forth horizontally.
     *
     * Uses static maps to track each entity initial position and movement direction.
     * Reverses direction when reaching the patrol distance boundary.
     *
     */
    inline void Update(Entity e, float dt)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(e)) return;

        auto& transform = reg.Get_Component<TransformComponent>(e);

        //multiple enemies can each have their own patrol data
        static std::unordered_map<Entity, float> startX;
        static std::unordered_map<Entity, bool> movingRight;

        const float patrolDistance = 200.0f;
        const float speed = 100.0f;
        //initializes patrol data the first time it runs
        if (startX.find(e) == startX.end())
        {
            startX[e] = transform.pos.x;
            movingRight[e] = true;
        }


        if (movingRight[e]) {
            transform.pos.x += speed * dt;
            if (transform.pos.x >= startX[e] + patrolDistance) {
                movingRight[e] = false;
            }
        }
        else {
            transform.pos.x -= speed * dt;
            if (transform.pos.x <= startX[e] - patrolDistance) {
                movingRight[e] = true;
            }
        }
    }


    /**
     * @brief Cleans up the PatrolEnemyScript when it ends.
     *
     * Logs a message indicating that the patrol behaviour has finished.
     *
     */
    inline void End(Entity e)
    {
        std::cout << "[PatrolEnemy] Patrol ended for entity " << e << std::endl;
    }
}





// ============================================
// REGISTER ALL SCRIPTS
// ============================================
inline void RegisterAllGameScripts()
{
    LogicSystem& logicSys = LogicSystem::Get();

    // Register Behaviour
    {
        BehaviourFCT ghoul;
        //function that takes an Entity e calls GhoulScript::Start(e)
        ghoul.Init = [](Entity e) { GhoulScript::Start(e); };
        ghoul.Update = [](Entity e, float dt) { GhoulScript::Update(e, dt); };
        ghoul.End = [](Entity e) { GhoulScript::End(e); };
        logicSys.RegisterScript("GhoulScript", ghoul, ScriptID::GHOUL);

        //same as ghoul
        BehaviourFCT patrol;
        patrol.Init = [](Entity e) { PatrolEnemyScript::Start(e); };
        patrol.Update = [](Entity e, float dt) { PatrolEnemyScript::Update(e, dt); };
        patrol.End = [](Entity e) { PatrolEnemyScript::End(e); };
        logicSys.RegisterScript("ManfredEnemy", patrol, ScriptID::PATROL_ENEMY);

    }

}