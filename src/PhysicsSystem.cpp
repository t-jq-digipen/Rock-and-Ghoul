#include "PhysicsSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Collision.h"
#include "Input.h"
#include "Remath.h"
#include <iostream>

// EntityManager& em;
//ComponentManager<TransformComponent>& transformManager;
//ComponentManager<PhysicsComponent>& physicsManager;

void PhysicsSystem::Initialize() {
    /*transformManager = CM_Transform();
    physicsManager = CM_Physics();*/
}

// Apply a damping force
/*
* F = m * a     | Force = Mass x Accel
* a = v / t     | Accel = speed / time
* 
* To modify velocity by damping, apply v = v * damp ^ dt
* 
*/
const float dampval = 0.05f; // Const decay value

// Maybe need to be specific value for each fella, idk
void Damping(Vec2& vec, float dt) {
    vec = Scale(vec, std::pow(dampval, dt));
}

void PhysicsSystem::Update(float dt) {

    // Step 1: Move entities

    for (auto& [entID, phyComp] : CM_Physics().getComponents())
    {
        // for Force force : ForceList(std::vector?)
        /*
        * {
        *   phyComp.vel.x += force.vel.x * dt;
        *   phyComp.vel.y += force.vel.y * dt;
        * }
        */
        // Apply forces

        // Apply damping
        Damping(phyComp.vel, dt);

        // Update positions
        if (Has_Component(entID, CT_Transform))
        {
            TransformComponent& transform = Get_TransformComponent(entID);
            transform.pos.x += phyComp.vel.x * dt;
            transform.pos.y += phyComp.vel.y * dt;
        }



    }




}

void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}
