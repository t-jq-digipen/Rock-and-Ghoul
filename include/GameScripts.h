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
    inline void Start(Entity gob)  // ADD inline
    {
        std::cout << "Ghoul script started for entity " << gob << std::endl;
    }

    inline void Update(Entity gob, float dt)  // ADD inline
    {
        Registry& registry = Registry::Get();

        if (registry.Has_Component<TransformComponent>(gob))
        {
            auto& transform = registry.Get_Component<TransformComponent>(gob);

            // Rotate 45 degrees per second
            transform.rot += 45.0f * dt;

            // Keep in 0-360 range
            if (transform.rot >= 360.0f) {
                transform.rot -= 360.0f;
            }
        }
    }

    inline void End(Entity gob)  // ADD inline
    {
        std::cout << "Ghoul script ended for entity " << gob << std::endl;
    }
}
// ============================================
// PATROL ENEMY SCRIPT
// ============================================
namespace PatrolEnemyScript
{
    inline void Start(Entity e)
    {
        std::cout << "[PatrolEnemy] Starting patrol for entity " << e << std::endl;

        // Store initial position as patrol start point
        Registry& reg = Registry::Get();
        if (reg.Has_Component<TransformComponent>(e)) {
            auto& transform = reg.Get_Component<TransformComponent>(e);
            // You could add a custom data component to store patrol points
            std::cout << "[PatrolEnemy] Starting position: (" << transform.pos.x << ", " << transform.pos.y << ")" << std::endl;
        }
    }

    //inline void Update(Entity e, float dt)
    //{
    //    Registry& reg = Registry::Get();

    //    if (!reg.Has_Component<TransformComponent>(e)) return;

    //    auto& transform = reg.Get_Component<TransformComponent>(e);

    //    // Simple back-and-forth patrol
    //    static float patrolDistance = 200.0f;
    //    static float speed = 100.0f;
    //    static float startX = transform.pos.x;
    //    static bool movingRight = true;

    //    // Move back and forth
    //    if (movingRight) {
    //        transform.pos.x += speed * dt;
    //        if (transform.pos.x >= startX + patrolDistance) {
    //            movingRight = false;
    //        }
    //    }
    //    else {
    //        transform.pos.x -= speed * dt;
    //        if (transform.pos.x <= startX - patrolDistance) {
    //            movingRight = true;
    //        }
    //    }
    //}
    inline void Update(Entity e, float dt)
    {
        Registry& reg = Registry::Get();
        if (!reg.Has_Component<TransformComponent>(e)) return;

        auto& transform = reg.Get_Component<TransformComponent>(e);

        // --- Store per-entity patrol data ---
        static std::unordered_map<Entity, float> startX;
        static std::unordered_map<Entity, bool> movingRight;

        const float patrolDistance = 200.0f;
        const float speed = 100.0f;

        if (startX.find(e) == startX.end())
        {
            startX[e] = transform.pos.x;
            movingRight[e] = true;
        }

        // --- Move back and forth ---
        if (movingRight[e]) {
            transform.pos.x += speed * dt;
            if (transform.pos.x >= startX[e] + patrolDistance)
                movingRight[e] = false;
        }
        else {
            transform.pos.x -= speed * dt;
            if (transform.pos.x <= startX[e] - patrolDistance)
                movingRight[e] = true;
        }
    }



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

    // Register Ghoul Behaviour
    {
        BehaviourFCT ghoul;
        ghoul.Init = [](Entity e) { GhoulScript::Start(e); };
        ghoul.Update = [](Entity e, float dt) { GhoulScript::Update(e, dt); };
        ghoul.End = [](Entity e) { GhoulScript::End(e); };
        logicSys.RegisterScript("GhoulScript", ghoul, ScriptID::GHOUL);

        BehaviourFCT patrol;
        patrol.Init = [](Entity e) { PatrolEnemyScript::Start(e); };
        patrol.Update = [](Entity e, float dt) { PatrolEnemyScript::Update(e, dt); };
        patrol.End = [](Entity e) { PatrolEnemyScript::End(e); };
        logicSys.RegisterScript("PatrolEnemy", patrol, ScriptID::PATROL_ENEMY);

    }

    //{
    //    BehaviourFCT patrolBehaviour;
    //    patrolBehaviour.Init = [](Entity e) { PatrolEnemyScript::Start(e); };
    //    patrolBehaviour.Update = [](Entity e, float dt) { PatrolEnemyScript::Update(e, dt); };
    //    patrolBehaviour.End = [](Entity e) { PatrolEnemyScript::End(e); };
    //    logicSys.RegisterScript("PatrolEnemy", patrolBehaviour);
    //    std::cout << "Registered PatrolEnemy" << std::endl;
    //}

    // Register more scripts here in the future
    /*
    {
        BehaviourFCT playerBehaviour;
        playerBehaviour.Init = [](Entity e) { PlayerScript::Start(e); };
        playerBehaviour.Update = [](Entity e, float dt) { PlayerScript::Update(e, dt); };
        playerBehaviour.End = [](Entity e) { PlayerScript::End(e); };
        logicSys.RegisterScript("PlayerScript", playerBehaviour);
        std::cout << "Registered PlayerScript" << std::endl;
    }
    */
}