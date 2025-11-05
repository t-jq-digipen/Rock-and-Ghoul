/* ========================================================
* @file     ForcesManager.cpp
* @author   Yasmin Tazi | yasmin.t@digipen.edu
* @date     04 NOV 2025
*
* @brief    Centralized force application system
*
* Copyright © 2025 DigiPen, All rights reserved.
* ========================================================
*/

#include "ForcesManager.h"
#include "Registry.h"
#include "Physics.h"

void ForcesManager::Initialize() {
    // Register default force profiles for each entity type

    // Player: Light drag
    RegisterProfile(T_PLAYER, [](Entity eID, float dt) {
        (void)dt;
        ApplyGravity(eID, 0.f);     // no gravity
        ApplyDrag(eID, 0.05f);         // light air resistance
        });

    // Enemy: Similar to player, more drag
    RegisterProfile(T_ENEMY, [](Entity eID, float dt) {
        (void)dt;
        ApplyGravity(eID, 0.f);
        ApplyDrag(eID, 0.08f);
        });

    // Projectile: Least drag
    RegisterProfile(T_PROJECTILE, [](Entity eID, float dt) {
        (void)dt;
        ApplyGravity(eID, 0.f);
        ApplyDrag(eID, 0.02f);         // less drag for projectiles
        });

    std::cout << "ForcesManager Initialized!\n";
}

void ForcesManager::Update(float dt) {
    Registry& reg = Registry::Get();

    // Apply force profiles to entities based on their tags
    for (auto& [eID, tagComp] : reg.Get_ComponentMap<TagComponent>()) {
        // Skip entities without physics
        if (!reg.Has_Component<Physics::PhysicsComponent>(eID)) {
            continue;
        }

        // Apply registered force profile if exists
        auto it = m_forceProfiles.find(tagComp.tag);
        if (it != m_forceProfiles.end()) {
            it->second(eID, dt);
        }
    }
}

void ForcesManager::Shutdown() {
    m_forceProfiles.clear();
}

void ForcesManager::RegisterProfile(C_TagID tag, ForceProfile profile) {
    m_forceProfiles[tag] = profile;
}

// ================================================================================
// STATIC FORCE HELPERS
// ================================================================================

void ForcesManager::ApplyGravity(Entity eID, float strength) {
    Registry& reg = Registry::Get();
    if (!reg.Has_Component<Physics::PhysicsComponent>(eID)) return;

    auto& physics = reg.Get_Component<Physics::PhysicsComponent>(eID);
    physics.AddForce({ 0.f, -1.f }, strength, 0.f); // Impulse gravity per frame
}

void ForcesManager::ApplyDrag(Entity eID, float coefficient) {
    Registry& reg = Registry::Get();
    if (!reg.Has_Component<Physics::PhysicsComponent>(eID)) return;

    auto& physics = reg.Get_Component<Physics::PhysicsComponent>(eID);

    Vec2 vel = physics.Get_Vel();
    float speed = RG_Length(vel);

    if (speed > 0.01f) {
        Vec2 dragDir = RG_Scale(vel, -1.f / speed); // opp dir
        float dragMag = coefficient * speed * speed; // quadratic drag
        physics.AddForce(dragDir, dragMag, 0.f);
    }
}

void ForcesManager::ApplyConstantForce(Entity eID, Vec2 direction, float magnitude) {
    Registry& reg = Registry::Get();
    if (!reg.Has_Component<Physics::PhysicsComponent>(eID)) return;

    auto& physics = reg.Get_Component<Physics::PhysicsComponent>(eID);
    physics.AddForce(direction, magnitude, 0.f);
}