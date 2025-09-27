#include "PhysicsSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Collision.h"
#include "Input.h"
#include <iostream>
#include "CollisionManager.h"
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

    // old
    /*for (Entity e = 0; e < em.maxEntities(); ++e) {
        if (!em.isAlive(e)) continue;
        if (!HasComponent<TransformComponent>(e) || !HasComponent<PhysicsComponent>(e)) continue;

        auto& transform = GetComponent<TransformComponent>(e);
        auto& physics = GetComponent<PhysicsComponent>(e);

        transform.pos.x += physics.vel.x * dt;
        transform.pos.y += physics.vel.y * dt;
    }*/

    // new
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
    collisionManager.Clear();
    collisionManager.CheckCollisions();
    collisionManager.ProcessEvents();
    // Step 2: Collision checks

    // old
    //for (Entity a = 0; a < em.maxEntities(); ++a) {
    //    if (!em.isAlive(a)) continue;
    //    if (!HasComponent<TransformComponent>(a) || !HasComponent<SpriteComponent>(a)) continue;

    //    auto& tA = GetComponent<TransformComponent>(a);
    //    auto& sA = GetComponent<SpriteComponent>(a);

    //    float halfW_A = sA.size.x * 0.5f * 0.6f;
    //    float halfH_A = sA.size.y * 0.5f * 0.2f;
    //    float offsetY_A = -140.0f;

    //    AABB boxA = {
    //        { tA.pos.x - halfW_A, tA.pos.y - halfH_A + offsetY_A },
    //        { tA.pos.x + halfW_A, tA.pos.y + halfH_A + offsetY_A }
    //    };

    //    // draw the hitbox if key '0' is held
    //    if (drawHitboxes) {
    //        float width = boxA.max.x - boxA.min.x;
    //        float height = boxA.max.y - boxA.min.y;
    //        renderer.DrawRect(
    //            { boxA.min.x + width * 0.5f, boxA.min.y + height * 0.5f }, // center
    //            width, height,
    //            { 1.0f, 0.0f, 0.0f } // red
    //        );
    //    }

    //    for (Entity b = a + 1; b < em.maxEntities(); ++b) {
    //        if (!em.isAlive(b)) continue;
    //        if (!HasComponent<TransformComponent>(b) || !HasComponent<SpriteComponent>(b)) continue;

    //        auto& tB = GetComponent<TransformComponent>(b);
    //        auto& sB = GetComponent<SpriteComponent>(b);

    //        float halfW_B = sB.size.x * 0.5f * 0.8f;
    //        float halfH_B = sB.size.y * 0.5f * 0.2f;
    //        float offsetY_B = -140.0f;

    //        AABB boxB = {
    //            { tB.pos.x - halfW_B, tB.pos.y - halfH_B + offsetY_B },
    //            { tB.pos.x + halfW_B, tB.pos.y + halfH_B + offsetY_B }
    //        };

    //        if (CollisionIntersection_RectRect_Static(boxA, boxB)) {
    //            std::cout << "Collision detected: " << a << " vs " << b << "\n";

    //            // Handle tags (player vs enemy)
    //            if (HasComponent<TagComponent>(a) && HasComponent<TagComponent>(b)) {
    //                auto& tagA = GetComponent<TagComponent>(a);
    //                auto& tagB = GetComponent<TagComponent>(b);

    //                if (tagA.tag == T_PLAYER && tagB.tag == T_ENEMY) {
    //                    if (HasComponent<HealthComponent>(a)) {
    //                        auto& health = GetComponent<HealthComponent>(a);
    //                        health.currhp -= 10.f;
    //                        std::cout << "Player took damage! HP = " << health.currhp << "\n";
    //                    }
    //                }
    //                if (tagA.tag == T_ENEMY && tagB.tag == T_PLAYER) {
    //                    if (HasComponent<HealthComponent>(b)) {
    //                        auto& health = GetComponent<HealthComponent>(b);
    //                        health.currhp -= 10.f;
    //                        std::cout << "Player took damage! HP = " << health.currhp << "\n";
    //                    }
    //                }
    //            }

    //            // Example response: stop entity A
    //            if (HasComponent<PhysicsComponent>(a)) {
    //                GetComponent<PhysicsComponent>(a).vel = { 0, 0 };
    //            }
    //        }
    //    }
    //}


}

void PhysicsSystem::Shutdown() {
    std::cout << "PhysicsSystem Shutdown!\n";
}
