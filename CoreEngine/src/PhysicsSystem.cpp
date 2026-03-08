/* ========================================================
 * @file	    PhysicsSystem.cpp
 * @author	    Tan Jun Quan       | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi        | yasmin.t@digipen.edu
 * @date	    19 NOV 2025
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
#include "CombatSystem.h"
#include "FrameRateController.h"
#include "RenderSystem.h"
 // extern reference to frame rate controller from CoreEngine
extern FrameRateController frameRate;
static CollisionManager collisionManager;


std::vector<Entity> expired{};

void QueueRemove(Entity eID) {
    expired.push_back(eID);
}

// EntityManager& em;
//ComponentManager<TransformComponent>& transformManager;
//ComponentManager<PhysicsComponent>& physicsManager;
void PhysicsSystem::Initialize() {
    std::cout << "PhysicsSystem Initialized!\n";

    CollisionLayerManager::Get().LoadCollisionSettings();
}

void PhysicsSystem::Update(float dt) {
    // dt is now fixedDt that is passed from CoreEngine

    static int debugCounter = 0;
    if (++debugCounter % 60 == 0) {
       // std::cout << "[PhysicsSystem] Update called, dt=" << dt << std::endl;
    }

    const bool* layerEnabled = RenderSystem::GetLayerEnabledArray();

    // Step 1: Move entities with physics
    for (auto& [entID, phyComp] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>())
    {
        // skip entities whose render layer is disabled
        if (m_registry.Has_Component<RenderComponent>(entID) &&
            !layerEnabled[m_registry.Get_Component<RenderComponent>(entID).GetRenderLayer()])
            continue;

        // Get transform component to multiply by fixed dt
        TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
        // Update physics component using fixed dt
        phyComp.Update(dt);
        // Update transform positions
        Vec2 deltaPos = RG_Scale(phyComp.Get_Vel(), dt);
        transComp.pos = RG_Add(transComp.pos, deltaPos);
    }

    // Step 2: Update collider hitboxes after all transforms moved
    for (auto& [entID, collider] : m_registry.Get_ComponentMap<ColliderComponent>())
    {
        // skip entities whose render layer is disabled
        if (m_registry.Has_Component<RenderComponent>(entID) &&
            !layerEnabled[m_registry.Get_Component<RenderComponent>(entID).GetRenderLayer()])
            continue;

        if (m_registry.Has_Component<TransformComponent>(entID)) {
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
            collider.UpdateHitboxes(transComp.pos, transComp.scale);
        }
    }

    for (auto& [entID, lifeComp] : m_registry.Get_ComponentMap<LifetimeComponent>())
    {
        // skip entities whose render layer is disabled (pause lifetime while hidden)
        if (m_registry.Has_Component<RenderComponent>(entID) &&
            !layerEnabled[m_registry.Get_Component<RenderComponent>(entID).GetRenderLayer()])
            continue;

        lifeComp.age += dt;
        if (lifeComp.age >= lifeComp.lifetime)
        {
            expired.push_back(entID);
        }
    }
    for (Entity eID : expired)
    {
        // Clean up projectile pool data before removing entity
        CombatSystem::Get().CleanupProjectilePoolData(eID);
        m_registry.Remove_Entity(eID);
    }
    expired.clear();

    // Collili checks!!(or different place?)
    // Collili reaction!!!
    collisionManager.Clear();
    collisionManager.CheckCollisions();
    collisionManager.ProcessEvents();
}

void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}