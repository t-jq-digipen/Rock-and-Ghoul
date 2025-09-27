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

    // Step 1: Move entities

    //for (auto& [entID, phyComp] : CM_Physics().getComponents())
    //{
    //    // for Force force : ForceList(std::vector?)
    //    /*
    //    * {
    //    *   phyComp.vel.x += force.vel.x * dt;
    //    *   phyComp.vel.y += force.vel.y * dt;
    //    * }
    //    */


    //    // Apply forces

    //    // Apply damping
    //    Damping(phyComp.vel, dt);

    //    // Update positions
    //    if (Has_Component(entID, CT_Transform))
    //    {
    //        TransformComponent& transform = Get_TransformComponent(entID);
    //        transform.pos.x += phyComp.vel.x * dt;
    //        transform.pos.y += phyComp.vel.y * dt;
    //    }



    //}

    // for new physics component
    for (auto& [entID, phyComp] : CM_Physics().getComponents())
    {
        // Get transform component (you gonna need it!)
        TransformComponent& transComp = Get_TransformComponent(entID);

        // Update physics component
        phyComp.Update();

        // Update transform positions
        transComp.pos = Add(transComp.pos, phyComp.Get_Vel());

        // updatehitboxes w transform position
        phyComp.UpdateHitboxes(transComp.pos);
        
        // Collili checks!!(or different place?)
        // Collili reaction!!!
    }



}

void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}
