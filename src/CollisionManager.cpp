/* ========================================================
 * @file        CollisionManager.cpp
 * @author      Yasmin Tazi        | yasmin.t@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        06 NOV 2025
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
#include "SpatialGrid.h"
#include "Remath.h"
#include <algorithm>

// Spatial grid for broad-phase optimization
static SpatialGrid spatialGrid(150.0f); // Cell size tuned for typical entity size

uint32_t CollisionLayerManager::RegisterLayer(const std::string& name) {
    if (layerBits.find(name) != layerBits.end()) { return layerBits[name]; }

    if (nextBit >= 32) 
    {
        std::cout << "Cannot exceed 32 collision layers! Failed to register " << name << '\n';
        return 0;
    }

    uint32_t bit = 1 << nextBit++;
    layerBits[name] = bit;
    layerNames[bit] = name;

    return bit;
}

uint32_t CollisionLayerManager::GetLayerBit(const std::string& name) const {
    auto it = layerBits.find(name);
    if (it == layerBits.end()) 
    { 
        std::cerr << "Collision layer " << name << " not registered!\n";
        return 0; 
    }

    return it->second;
}

std::string CollisionLayerManager::GetLayerName(uint32_t bit) const {
    auto it = layerNames.find(bit);
    if (it == layerNames.end())
    {
        std::cerr << "Collision layer bit " << bit << " not registered!\n";
        return "";
    }

    return it->second;
}

bool CollisionLayerManager::ShouldCollide(uint32_t layerA, uint32_t maskA, uint32_t layerB, uint32_t maskB) const {
    return (maskA & layerB) || (maskB & layerA);
}

void CollisionLayerManager::Clear() {
    layerBits.clear();
    layerNames.clear();
    nextBit = 0;
}

void CollisionManager::Clear() {
    events.clear();
}

void CollisionManager::AddEvent(Entity a, Entity b) {
    events.push_back({ a, b });
}

void CollisionManager::ProcessEvents() {
    //loops through each stored collision event from this frame.
    //for (auto& e : events) {
    //    //if both entities have a tag
    //    //tags tell us what kind of entity
    //    if (m_registry.Has_Component<TagComponent>(e.a) && m_registry.Has_Component<TagComponent>(e.b)) {
    //        auto& tagA = m_registry.Get_Component<TagComponent>(e.a);
    //        auto& tagB = m_registry.Get_Component<TagComponent>(e.b);

    //        std::cout << "Collision detected between " << TagToString(tagA.tag) << " (Entity " << e.a << ")" << " and "  << TagToString(tagB.tag) << " (Entity " << e.b << ")" << std::endl;
    //    }
    //    else {
    //        std::cout << "Collision detected between Entity " << e.a << " and Entity " << e.b<< " (no tags)" << std::endl;
    //    }
    //}
    Clear();
}

//void CollisionManager::ActionStop(Entity a, Entity b) {
//    auto& transA = m_registry.Get_Component<TransformComponent>(a);
//    auto& physA = m_registry.Get_Component<Physics::PhysicsComponent>(a);
//    auto& transB = m_registry.Get_Component<TransformComponent>(b);
//    auto& physB = m_registry.Get_Component<Physics::PhysicsComponent>(b);
//
//    const AABB& aabbA = physA.Get_AABB();
//    const AABB& aabbB = physB.Get_AABB();
//
//    //overlap
//    Vec2 diff = RG_Subtract(transB.pos, transA.pos);
//    float overlapX = (aabbA.dim.x + aabbB.dim.x) / 2.0f - fabs(diff.x);
//    float overlapY = (aabbA.dim.y + aabbB.dim.y) / 2.0f - fabs(diff.y);
//
//    if (overlapX <= 0 || overlapY <= 0) return;
//
//    //  Separate along the axis of least overlap
//    //both objects are movable, split the correction between them (so neither gets unfairly displaced)
//    if (overlapX < overlapY) {
//        float halfOverlap = overlapX / 2.0f;
//        if (diff.x > 0) {
//            transA.pos.x -= halfOverlap;
//            transB.pos.x += halfOverlap;
//        }
//        else {
//            transA.pos.x += halfOverlap;
//            transB.pos.x -= halfOverlap;
//        }
//    }
//    else {
//        float halfOverlap = overlapY / 2.0f;
//        if (diff.y > 0) {
//            transA.pos.y -= halfOverlap;
//            transB.pos.y += halfOverlap;
//        }
//        else {
//            transA.pos.y += halfOverlap;
//            transB.pos.y -= halfOverlap;
//        }
//    }
//
//    physA.UpdateHitboxes(transA.pos);
//    physB.UpdateHitboxes(transB.pos);
//
//    // stop velocities
//    physA.Set_Vel({ 0.0f, 0.0f });
//    physB.Set_Vel({ 0.0f, 0.0f });
//}

//void CollisionManager::ActionStop_Static(Entity dynamic, Entity staticObj) {
//    auto& transDyn = m_registry.Get_Component<TransformComponent>(dynamic);
//    auto& physDyn = m_registry.Get_Component<Physics::PhysicsComponent>(dynamic);
//    auto& transStatic = m_registry.Get_Component<TransformComponent>(staticObj);
//    auto& physStatic = m_registry.Get_Component<Physics::PhysicsComponent>(staticObj);
//
//    const AABB& aabbDyn = physDyn.Get_AABB();
//    const AABB& aabbStatic = physStatic.Get_AABB();
//
//    if (!CollisionIntersection_RectRect_Static(aabbDyn, aabbStatic)) {
//        return;
//    }
//
//
//    //Lecture 6_2 and 7
//    //if (leftA > rightB) then no collision
//    //if (leftB > rightA) then no collision
//    //if (topA < bottomB) then no collision
//    //if (topB < bottomA) then no collision
//    Vec2 diff = RG_Subtract(transDyn.pos, transStatic.pos);
//    float overlapX = (aabbDyn.dim.x + aabbStatic.dim.x) / 2.0f - fabs(diff.x);
//    float overlapY = (aabbDyn.dim.y + aabbStatic.dim.y) / 2.0f - fabs(diff.y);
//
//    // collision normal
//    Vec2 normal;
//    if (overlapX < overlapY) {
//        normal = (diff.x > 0) ? Vec2{ 1.0f, 0.0f } : Vec2{ -1.0f, 0.0f };
//    }
//    else {
//        normal = (diff.y > 0) ? Vec2{ 0.0f, 1.0f } : Vec2{ 0.0f, -1.0f };
//    }
//
//    float minimum_overlap = std::min(overlapX, overlapY);
//    Vec2 displacement = RG_Scale(normal, minimum_overlap);
//    transDyn.pos = RG_Add(transDyn.pos, displacement);
//
//    physDyn.UpdateHitboxes(transDyn.pos);
//
//    //Lecture 16 - Animated Point to Line
//    Vec2 velDyn = physDyn.Get_Vel();
//    float velocitynorm = RG_DotProduct(velDyn, normal);
//    // If moving into the wall (negative dot product)
//    if (velocitynorm < 0) {
//        // Remove the normal component of velocity
//        Vec2 correctionstep = RG_Scale(normal, velocitynorm);
//        physDyn.Set_Vel(RG_Subtract(velDyn, correctionstep));
//    }
//}

// IMPULSE-BASED COLLISION (LINEAR PHYSICS SYSTEM)
void CollisionManager::ActionResolve_ImpulseBased(Entity a, Entity b) {
    auto& transA = m_registry.Get_Component<TransformComponent>(a);
    auto& physA = m_registry.Get_Component<Physics::PhysicsComponent>(a);
    auto& transB = m_registry.Get_Component<TransformComponent>(b);
    auto& physB = m_registry.Get_Component<Physics::PhysicsComponent>(b);

    const AABB& aabbA = physA.Get_AABB();
    const AABB& aabbB = physB.Get_AABB();

    // Calculate collision normal
    Vec2 diff = RG_Subtract(transB.pos, transA.pos);
    float overlapX = (aabbA.dim.x + aabbB.dim.x) / 2.0f - RG_Abs(diff.x);
    float overlapY = (aabbA.dim.y + aabbB.dim.y) / 2.0f - RG_Abs(diff.y);

    if (overlapX <= 0 || overlapY <= 0) return;

    // Find collision normal
    Vec2 normal;
    float penetrationDepth;

    if (overlapX < overlapY) {
        normal = (diff.x > 0) ? Vec2{ 1.0f, 0.0f } : Vec2{ -1.0f, 0.0f };
        penetrationDepth = overlapX;
    }
    else {
        normal = (diff.y > 0) ? Vec2{ 0.0f, 1.0f } : Vec2{ 0.0f, -1.0f };
        penetrationDepth = overlapY;
    }

    // minimal position correction to prevent sinking
    bool aStatic = physA.IsStatic();
    bool bStatic = physB.IsStatic();

    const float separationBuffer = 0.1f;  // minimal buffer to prevent sinking

    float correctionAmount = penetrationDepth + separationBuffer;

    if (!aStatic && !bStatic) {
        float half = correctionAmount / 2.0f;
        Vec2 correction = RG_Scale(normal, half);
        transA.pos = RG_Subtract(transA.pos, correction);
        transB.pos = RG_Add(transB.pos, correction);
        physA.UpdateHitboxes(transA.pos);
        physB.UpdateHitboxes(transB.pos);
    }
    else if (!aStatic) {
        Vec2 correction = RG_Scale(normal, -correctionAmount);
        transA.pos = RG_Add(transA.pos, correction);
        physA.UpdateHitboxes(transA.pos);
    }
    else if (!bStatic) {
        Vec2 correction = RG_Scale(normal, correctionAmount);
        transB.pos = RG_Add(transB.pos, correction);
        physB.UpdateHitboxes(transB.pos);
    }

        // Calculate relative velocity
        Vec2 velA = physA.Get_Vel();
        Vec2 velB = physB.Get_Vel();
        Vec2 relativeVel = RG_Subtract(velA, velB);
        float velAlongNormal = RG_DotProduct(relativeVel, normal);

        // Don't resolve if separating
        if (velAlongNormal > -0.1f) return;

        // Special handling for high-speed impacts (like dashing into walls)
        const float highSpeedThreshold = 500.0f;
        if ((aStatic || bStatic) && RG_Abs(velAlongNormal) > highSpeedThreshold) {
            // For high-speed wall impacts, just stop velocity along normal
            if (!aStatic) {
                Vec2 normalComponent = RG_Scale(normal, velAlongNormal);
                velA = RG_Subtract(velA, normalComponent);
                physA.Set_Vel(velA);
            }
            if (!bStatic) {
                Vec2 normalComponent = RG_Scale(normal, -velAlongNormal);
                velB = RG_Add(velB, normalComponent);
                physB.Set_Vel(velB);
            }
            return;  // Skip impulse calculation for high-speed wall impacts
        }

        float e = RG_Min(physA.GetRestitution(), physB.GetRestitution());

        float invMassA = aStatic ? 0.0f : 1.0f / physA.GetMass();
        float invMassB = bStatic ? 0.0f : 1.0f / physB.GetMass();

        // Preventing division by zero
        if (invMassA + invMassB < 0.0001f) return;

        // Calculate impulse magnitude j = -(1+e)*v_rel / (1/m_a + 1/m_b)
        float j = -(1.0f + e) * velAlongNormal / (invMassA + invMassB);
        Vec2 impulse = RG_Scale(normal, j);

        // Apply impulses, changes velocity, not position
        if (!aStatic) {
            physA.ApplyLinearImpulse(impulse);
        }
        if (!bStatic) {
            physB.ApplyLinearImpulse(RG_Scale(impulse, -1.0f));
        }
}

// Replaced ActionStop with Impulse method
void CollisionManager::CollisionActions(Entity a, Entity b, bool intersected)
{
    if (!intersected) return;

    AddEvent(a, b);

    ActionResolve_ImpulseBased(a, b);
}

void CollisionManager::CheckCollisions() {
    // Clear collision markers from previous frame FIRST (before clearing grid)
    spatialGrid.ClearCollisionMarkers();

    auto& physicsComps = m_registry.Get_ComponentMap<Physics::PhysicsComponent>();

    // Clear and rebuild spatial grid AFTER we've used the old data for rendering
    spatialGrid.Clear();

    // BROAD PHASE: Build spatial grid
    for (auto& [entity, phyComp] : physicsComps) {
        if (!phyComp.CollisionActive() || !m_registry.Has_Component<TransformComponent>(entity)) {
            continue;
        }

        auto& trans = m_registry.Get_Component<TransformComponent>(entity);
        phyComp.UpdateHitboxes(trans.pos);

        // Insert into spatial grid
        spatialGrid.Insert(entity, phyComp.Get_AABB());
    }

    // NARROW PHASE: Get potential pairs from spatial grid and check them
    auto potentialPairs = spatialGrid.GetAllPotentialPairs();

    for (const auto& [a, b] : potentialPairs) {
        // Skip self-collision
        if (a == b) {
            continue;
        }

        // Validate both entities still exist and have required components
        if (!m_registry.Has_Component<Physics::PhysicsComponent>(a) ||
            !m_registry.Has_Component<Physics::PhysicsComponent>(b)) {
            continue;
        }

        auto& compA = m_registry.Get_Component<Physics::PhysicsComponent>(a);
        auto& compB = m_registry.Get_Component<Physics::PhysicsComponent>(b);

        if (!compA.CollisionActive() || !compB.CollisionActive()) {
            continue;
        }

        // Skip wall-wall collisions obviously
        if (m_registry.Has_Component<TagComponent>(a) && m_registry.Has_Component<TagComponent>(b)) {
            auto& tagA = m_registry.Get_Component<TagComponent>(a);
            auto& tagB = m_registry.Get_Component<TagComponent>(b);
            if (tagA.tag == T_WALL && tagB.tag == T_WALL) {
                continue;
            }
        }

        // Determine collision shapes
        bool aIsCircle = false, bIsCircle = false;

        if (m_registry.Has_Component<ShapeComponent>(a)) {
            auto& shapeA = m_registry.Get_Component<ShapeComponent>(a);
            aIsCircle = (shapeA.shape == SHAPE_CIRCLE);
        }
        if (m_registry.Has_Component<ShapeComponent>(b)) {
            auto& shapeB = m_registry.Get_Component<ShapeComponent>(b);
            bIsCircle = (shapeB.shape == SHAPE_CIRCLE);
        }

        // NARROW PHASE: Precise collision detection
        bool hit = false;

        if (!aIsCircle && !bIsCircle) {
            // Rect-Rect
            hit = CollisionIntersection_RectRect_Static(compA.Get_AABB(), compB.Get_AABB());
        }
        else if (aIsCircle && bIsCircle) {
            // Circle-Circle
            hit = CollisionIntersection_CC_Static(compA.Get_Circle(), compB.Get_Circle());
        }
        else {
            // Circle-Rect
            hit = aIsCircle ?
                CollisionIntersection_CA_Static(compA.Get_Circle(), compB.Get_AABB()) :
                CollisionIntersection_CA_Static(compB.Get_Circle(), compA.Get_AABB());
        }

        if (hit) {
            // Mark cells where collision occurred (intersection region only)
            spatialGrid.MarkCollisionBetween(compA.Get_AABB(), compB.Get_AABB());

            CollisionActions(a, b, true);
        }
    }
}

SpatialGrid* CollisionManager::GetSpatialGrid() {
    return &spatialGrid;
}
