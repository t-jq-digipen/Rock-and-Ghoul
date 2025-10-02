/* ========================================================
 * @file    CollisionManager.cpp
 * @author   w.shiyuregine@digipen.edu
 * @date    01/10/2025
 *
 * @brief
 *
 * This system iterates over all entities that have both Physics and Transform
 * components and performs collision detection between their collision.
 *
 * When an overlap is detected, a CollisionEvent is created and stored for
 * later processing.
 *
 *   - Position correction: computes the penetration depth and displaces the dynamic obj
 *      along the collision normal so shapes no longer overlap.
 *   - Velocity correction: projects the obj velocity onto the normal and removes any inward component, preventing continued motion into
 *     the static object.
 *
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

            //std::cout << "Collision detected between " << TagToString(tagA.tag) << " (Entity " << e.a << ")" << " and "  << TagToString(tagB.tag) << " (Entity " << e.b << ")" << std::endl;
        }
        else {
            //std::cout << "Collision detected between Entity " << e.a << " and Entity " << e.b<< " (no tags)" << std::endl;
        }
    }
    Clear();
}

void CollisionManager::ActionStop(Entity a, Entity b) {
    auto& transA = Get_TransformComponent(a);
    auto& physA = Get_PhysicsComponent(a);
    auto& transB = Get_TransformComponent(b);
    auto& physB = Get_PhysicsComponent(b);

    const AABB& aabbA = physA.Get_AABB();
    const AABB& aabbB = physB.Get_AABB();

    //overlap
    Vec2 diff = RG_Subtract(transB.pos, transA.pos);
    float overlapX = (aabbA.dim.x + aabbB.dim.x) / 2.0f - fabs(diff.x);
    float overlapY = (aabbA.dim.y + aabbB.dim.y) / 2.0f - fabs(diff.y);

    if (overlapX <= 0 || overlapY <= 0) return;

    //  Separate along the axis of least overlap
    //both objects are movable, split the correction between them (so neither gets unfairly displaced)
    if (overlapX < overlapY) {
        float halfOverlap = overlapX / 2.0f;
        if (diff.x > 0) {
            transA.pos.x -= halfOverlap;
            transB.pos.x += halfOverlap;
        }
        else {
            transA.pos.x += halfOverlap;
            transB.pos.x -= halfOverlap;
        }
    }
    else {
        float halfOverlap = overlapY / 2.0f;
        if (diff.y > 0) {
            transA.pos.y -= halfOverlap;
            transB.pos.y += halfOverlap;
        }
        else {
            transA.pos.y += halfOverlap;
            transB.pos.y -= halfOverlap;
        }
    }

    physA.UpdateHitboxes(transA.pos);
    physB.UpdateHitboxes(transB.pos);

    // stop velocities
    physA.Set_Vel({ 0.0f, 0.0f });
    physB.Set_Vel({ 0.0f, 0.0f });
}

void CollisionManager::ActionStop_Static(Entity dynamic, Entity staticObj) {
    auto& transDyn = Get_TransformComponent(dynamic);
    auto& physDyn = Get_PhysicsComponent(dynamic);
    auto& transStatic = Get_TransformComponent(staticObj);
    auto& physStatic = Get_PhysicsComponent(staticObj);

    const AABB& aabbDyn = physDyn.Get_AABB();
    const AABB& aabbStatic = physStatic.Get_AABB();

    if (!CollisionIntersection_RectRect_Static(aabbDyn, aabbStatic)) {
        return;
    }


    //Lecture 6_2 and 7
    //if (leftA > rightB) then no collision
    //if (leftB > rightA) then no collision
    //if (topA < bottomB) then no collision
    //if (topB < bottomA) then no collision
    Vec2 diff = RG_Subtract(transDyn.pos, transStatic.pos);
    float overlapX = (aabbDyn.dim.x + aabbStatic.dim.x) / 2.0f - fabs(diff.x);
    float overlapY = (aabbDyn.dim.y + aabbStatic.dim.y) / 2.0f - fabs(diff.y);

    // collision normal
    Vec2 normal;
    if (overlapX < overlapY) {
        normal = (diff.x > 0) ? Vec2{ 1.0f, 0.0f } : Vec2{ -1.0f, 0.0f };
    }
    else {
        normal = (diff.y > 0) ? Vec2{ 0.0f, 1.0f } : Vec2{ 0.0f, -1.0f };
    }

    float minimum_overlap = std::min(overlapX, overlapY);
    Vec2 displacement = RG_Scale(normal, minimum_overlap);
    transDyn.pos = RG_Add(transDyn.pos, displacement);

    physDyn.UpdateHitboxes(transDyn.pos);

    //Lecture 16 – Animated Point to Line
    Vec2 velDyn = physDyn.Get_Vel();
    float velocitynorm = RG_DotProduct(velDyn, normal);
    // If moving into the wall (negative dot product)
    if (velocitynorm < 0) {
        // Remove the normal component of velocity
        Vec2 correctionstep = RG_Scale(normal, velocitynorm);
        physDyn.Set_Vel(RG_Subtract(velDyn, correctionstep));
    }
}

void CollisionManager::CollisionActions(Entity a, Entity b, bool intersected)
{
    if (!intersected) return;

    AddEvent(a, b);

    auto& compA = Get_PhysicsComponent(a);
    auto& compB = Get_PhysicsComponent(b);

    if (compA.IsStatic() && !compB.IsStatic()) {
        ActionStop_Static(b, a);
    }
    else if (!compA.IsStatic() && compB.IsStatic()) {
        ActionStop_Static(a, b);
    }
    else {
        ActionStop(a, b);
    }
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

            // collision shapes
            bool aIsCircle = false, bIsCircle = false;

            if (Has_Component(a, CT_Shape)) {
                auto& shapeA = Get_ShapeComponent(a);
                aIsCircle = (shapeA.shape == SHAPE_CIRCLE);
            }
            if (Has_Component(b, CT_Shape)) {
                auto& shapeB = Get_ShapeComponent(b);
                bIsCircle = (shapeB.shape == SHAPE_CIRCLE);
            }

            if (Has_Component(a, CT_Tag) && Has_Component(b, CT_Tag)) {
                auto& tagA = Get_TagComponent(a);
                auto& tagB = Get_TagComponent(b);
                if (tagA.tag == T_WALL && tagB.tag == T_WALL) {
                    continue; // don’t process wall–wall collisions
                }
            }

            if (!aIsCircle && !bIsCircle) {
                // Both are rectangles
                bool hit = CollisionIntersection_RectRect_Static(compA.Get_AABB(), compB.Get_AABB());
                CollisionActions(a, b, hit);
            }

            // === CIRCLE - CIRCLE ===
            else if (aIsCircle && bIsCircle) {
                // Both are circles
                bool hit = CollisionIntersection_CC_Static(compA.Get_Circle(), compB.Get_Circle());
                CollisionActions(a, b, hit);
            }

            // === CIRCLE - RECT ===
            else {
                // One is circle, other is rectangle
                bool hit = aIsCircle? CollisionIntersection_CA_Static(compA.Get_Circle(), compB.Get_AABB()) : CollisionIntersection_CA_Static(compB.Get_Circle(), compA.Get_AABB());

                CollisionActions(a, b, hit);
            }
        }
    }
}
