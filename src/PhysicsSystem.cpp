/* ========================================================
 * @file	    PhysicsSystem.cpp
 * @author	    Tan Jun Quan | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date	    29 SEP 2025
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
// EntityManager& em;
//ComponentManager<TransformComponent>& transformManager;
//ComponentManager<PhysicsComponent>& physicsManager;

void PhysicsSystem::Initialize() {
    /*transformManager = CM_Transform();
    physicsManager = CM_Physics();*/
}


void PhysicsSystem::Update(float dt) {
    (void)dt;
    // Step 1: Move entities


    // for new physics component
    for (auto& [entID, phyComp] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>())
    {
        // Get transform component (you gonna need it!)
        TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);

        // Update physics component (applies forces, damping...)
        phyComp.Update(dt);

        // Update transform positions
        transComp.pos = RG_Add(transComp.pos, phyComp.Get_Vel());

        // updatehitboxes w transform position
        phyComp.UpdateHitboxes(transComp.pos);
        
        // Collili checks!!(or different place?)
        // Collili reaction!!!
        collisionManager.Clear();
        collisionManager.CheckCollisions();
        collisionManager.ProcessEvents();
    }

}

void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}
