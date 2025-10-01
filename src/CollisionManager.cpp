/* ========================================================
 * @file    CollisionManager.cpp
 * @author   w.shiyuregine@digipen.edu
 * @date    01/10/2025
 *
 * @brief
 *
 * This checks all entities with Physics and Transform components
 * for overlaps between their collision shapes (rectangles and circles).
 * Detected collisions are stored as events for later processing.
 *
 * The default resolution strategy (ActionStop) rolls entities back to their
 * previous positions and zeroes their velocity, ensuring they do not overlap, can be improved
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "CollisionManager.h"

void CollisionManager::Clear() {
    events.clear();
}

void CollisionManager::AddEvent(Entity a, Entity b) {
    events.push_back({ a, b });
}

void CollisionManager::ProcessEvents() {
    //loops through each stored collision event from this frame.
    for (auto& e : events) {
        //if both entities have a tag
        //tags tell us what kind of entity
        if (Has_Component(e.a, CT_Tag) && Has_Component(e.b, CT_Tag)) {
            auto& tagA = Get_TagComponent(e.a);
            auto& tagB = Get_TagComponent(e.b);

            std::cout << "Collision detected between " << TagToString(tagA.tag) << " (Entity " << e.a << ")" << " and "  << TagToString(tagB.tag) << " (Entity " << e.b << ")" << std::endl;
        }
        else {
            std::cout << "Collision detected between Entity " << e.a << " and Entity " << e.b<< " (no tags)" << std::endl;
        }
    }
    Clear();
}

void CollisionManager::ActionStop(Entity a, Entity b) {
    auto& transA = Get_TransformComponent(a);
    auto& physA = Get_PhysicsComponent(a);
    auto& transB = Get_TransformComponent(b);
    auto& physB = Get_PhysicsComponent(b);

    // rollback
    transA.pos = transA.Prevpos;
    transB.pos = transB.Prevpos;

    // stop
    physA.Set_Vel({ 0.f, 0.f });
    physB.Set_Vel({ 0.f, 0.f });
}


void CollisionManager::CheckCollisions() {
    auto& physicsComps = CM_Physics().getComponents();

    for (auto itA = physicsComps.begin(); itA != physicsComps.end(); ++itA) {
        Entity a = itA->first;
        Physics::PhysicsComponent& compA = itA->second;

        if (!compA.CollisionActive() || !Has_Component(a, CT_Transform)) continue;

        auto& transA = Get_TransformComponent(a);
        compA.UpdateHitboxes(transA.pos);

        for (auto itB = std::next(itA); itB != physicsComps.end(); ++itB) {
            Entity b = itB->first;
            Physics::PhysicsComponent& compB = itB->second;

            if (!compB.CollisionActive() || !Has_Component(b, CT_Transform)) continue;

            auto& transB = Get_TransformComponent(b);
            compB.UpdateHitboxes(transB.pos);

            // collision shapes based on sprite textureID
            bool aIsCircle = false, bIsCircle = false;

            // JQ COMMENT OUT HAHAHA
            if (Has_Component(a, CT_Shape)) {
                auto& shapeA = Get_ShapeComponent(a);
                aIsCircle = (shapeA.shape == SHAPE_CIRCLE);
            }

            if (Has_Component(b, CT_Shape)) {
                auto& shapeB = Get_ShapeComponent(b);
                bIsCircle = (shapeB.shape == SHAPE_CIRCLE);
            }
            
            // collision n action
            if (!aIsCircle && !bIsCircle) {
                if (CollisionIntersection_RectRect_Static(compA.Get_AABB(), compB.Get_AABB())) {
                    AddEvent(a, b);
                    ActionStop(a, b);
                }
            }
            else if (aIsCircle && bIsCircle) {
                if (CollisionIntersection_CC_Static(compA.Get_Circle(), compB.Get_Circle())) {
                    AddEvent(a, b);
                    ActionStop(a, b);
                }
            }
            else {
                if (aIsCircle) {
                    if (CollisionIntersection_CA_Static(compA.Get_Circle(), compB.Get_AABB())) {
                        AddEvent(a, b);
                        ActionStop(a, b);
                    }
                }
                else {
                    if (CollisionIntersection_CA_Static(compB.Get_Circle(), compA.Get_AABB())) {
                        AddEvent(a, b);
                        ActionStop(a, b);
                    }
                }
            }
        }
    }
}