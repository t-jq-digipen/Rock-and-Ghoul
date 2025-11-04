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
 * Copyright � 2025 DigiPen, All rights reserved.
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
#include "FrameRateController.h"
 // extern reference to frame rate controller from CoreEngine
extern FrameRateController frameRate;
static CollisionManager collisionManager;

// EntityManager& em;
//ComponentManager<TransformComponent>& transformManager;
//ComponentManager<PhysicsComponent>& physicsManager;
void PhysicsSystem::Initialize() {
    std::cout << "PhysicsSystem Initialized!\n";
}
void PhysicsSystem::Update(float dt) {
    // dt is now fixedDt that is passed from CoreEngine

    // getting number of fixed timesteps to execute this frame
    int numSteps = frameRate.GetCurrentNumberOfSteps();
    
    // loop for each fixed timestep that needs to be simulated
    for (int step = 0; step < numSteps; ++step)
    {
        // Step 1: Move entities
        // for new physics component
        for (auto& [entID, phyComp] : m_registry.Get_ComponentMap<Physics::PhysicsComponent>())
        {
            // Get transform component to multiply by fixed dt
            TransformComponent& transComp = m_registry.Get_Component<TransformComponent>(entID);
            // Update physics component using fixed dt
            phyComp.Update();
            // Update transform positions
            Vec2 deltaPos = RG_Scale(phyComp.Get_Vel(), dt);
            transComp.pos = RG_Add(transComp.pos, deltaPos);
            // updatehitboxes w transform position
            phyComp.UpdateHitboxes(transComp.pos);
            // Collili checks!!(or different place?)
            // Collili reaction!!!
            collisionManager.Clear();
            collisionManager.CheckCollisions();
            collisionManager.ProcessEvents();
        }
    }
}
void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}