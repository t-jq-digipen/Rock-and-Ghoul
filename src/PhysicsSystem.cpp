#include "PhysicsSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Collision.h"
#include "Input.h"
#include <iostream>

// EntityManager& em;
//ComponentManager<TransformComponent>& transformManager;
//ComponentManager<PhysicsComponent>& physicsManager;

void PhysicsSystem::Initialize() {
    /*transformManager = CM_Transform();
    physicsManager = CM_Physics();*/
}


void PhysicsSystem::Update(float dt) {

    // Step 1: Move entities

    for (auto& [entID, phyComp] : CM_Physics().getComponents())
    {
        if (Has_Component(entID, CT_Transform))
        {
            TransformComponent& transform = Get_TransformComponent(entID);
            transform.pos.x += phyComp.vel.x * dt;
            transform.pos.y += phyComp.vel.y * dt;
        }
    }

    // Check if should draw hitboxes
    bool drawHitboxes = Input::IsKeyHeld(GLFW_KEY_0);

    // Step 2: Collision checks


}

void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}
