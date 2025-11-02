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
// ADD MORE SCRIPTS HERE AS NEEDED
// ============================================
// Example: Player script, Enemy script, etc.
/*
namespace PlayerScript
{
    inline void Start(Entity e)
    {
        std::cout << "Player script started" << std::endl;
    }

    inline void Update(Entity e, float dt)
    {
        // Player logic here
    }

    inline void End(Entity e)
    {
        std::cout << "Player script ended" << std::endl;
    }
}
*/

// ============================================
// REGISTER ALL SCRIPTS
// ============================================
inline void RegisterAllGameScripts()
{
    LogicSystem& logicSys = LogicSystem::Get();

    // Register Ghoul Behaviour
    {
        BehaviourFCT ghoulBehaviour;

        ghoulBehaviour.Init = [](Entity e) {
            GhoulScript::Start(e);
            };

        ghoulBehaviour.Update = [](Entity e, float dt) {
            GhoulScript::Update(e, dt);
            };

        ghoulBehaviour.End = [](Entity e) {
            GhoulScript::End(e);
            };

        logicSys.RegisterScript("GhoulScript", ghoulBehaviour);
        std::cout << "Registered GhoulScript" << std::endl;
    }

    // Register more scripts here in the future
    // Example:
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