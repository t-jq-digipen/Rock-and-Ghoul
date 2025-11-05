/* ========================================================
 * @file	    PhysicsSystem.cpp
 * @author	    Tan Jun Quan       | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi        | yasmin.t@digipen.edu
 * @date	    31 OCT 2025
 *
 * @brief	Physics system for ECS
 *
 * This file implements the update order of the various
 * parts in the physics component, with reference to other
 * components such as Transform.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "PhysicsSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Collision.h"
#include "Input.h"
#include "Remath.h"
#include <iostream>
#include "CollisionManager.h"
#include "Physics.h"

static CollisionManager collisionManager;

void PhysicsSystem::Initialize() {
    std::cout << "PhysicsSystem Initialized!\n";
}

void PhysicsSystem::Update(float dt) {
    // Clamp dt to prevent issues
    const float maxDt = 0.033f;
    if (dt > maxDt) {
        dt = maxDt;
    }

    // STEP 1: Update physics (forces -> velocities)
    for (auto& [entID, phyComp] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {
        if (!m_registry.Has_Component<TransformComponent>(entID)) {
            continue;
        }

        // Update physics component
        phyComp.Update(dt);
    }

    // STEP 2: Integrate velocities into positions
    for (auto& [entID, phyComp] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>()) {
        if (!m_registry.Has_Component<TransformComponent>(entID)) {
            continue;
        }

        TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);

        // Move entity
        phyComp.IntegrateVelocity(transComp.pos, dt);

        // Update hitboxes
        phyComp.UpdateHitboxes(transComp.pos);
    }

    // STEP 3: Check and resolve collisions
    collisionManager.Clear();
    collisionManager.CheckCollisions();
    collisionManager.ProcessEvents();
}

void PhysicsSystem::Shutdown() {
}