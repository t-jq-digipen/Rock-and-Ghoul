/* ========================================================
 * @file        CollisionManager.cpp
 * @author      Yasmin Tazi        | yasmin.t@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        24 NOV 2025
 *
 * @brief
 *
 * This system iterates over all entities that have Collider and Transform
 * components and performs collision detection using a spatial grid for
 * broad-phase optimization and collision layers for filtering.
 *
 * When an overlap is detected, a CollisionEvent is created and stored for
 * later processing.
 *
 *   - Collision layers: entities are filtered by layer/mask to determine
 *     which pairs should collide (e.g. Projectile vs Enemy).
 *   - Position correction: computes the penetration depth and displaces the
 *     dynamic obj along the collision normal so shapes no longer overlap.
 *   - Velocity correction: projects the obj velocity onto the normal and
 *     removes any inward component, preventing continued motion into the
 *     static object.
 *   - Trigger collisions: when a trigger collider with a triggerEvent set
 *     collides with another entity, publishes a TriggerEvent to the EventBus
 *     and removes the trigger's collider so it only fires once (projectiles).
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "CollisionManager.h"
#include "SpatialGrid.h"
#include "Remath.h"
#include "Component.h"
#include "EntityManager.h"
#include <algorithm>
#include "JsonLoad.h"
#include "FileIO.h"
#include "FilePath.h"

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
    layerMasks[bit] = 0;

    return bit;
}

void CollisionLayerManager::LoadCollisionSettings() {

    std::string settings = FileIO::ReadFile(FilePath::COLLISIONSETTINGS_FILE);

    if (settings.empty())
    {
        uint32_t layerBit;
        layerBit = GetLayerBit("Player");
        layerMasks[layerBit] |= GetLayerBit("Enemy");
        layerMasks[layerBit] |= GetLayerBit("Wall");
        layerBit = GetLayerBit("Enemy");
        layerMasks[layerBit] |= GetLayerBit("Player");
        layerMasks[layerBit] |= GetLayerBit("Enemy");
        layerMasks[layerBit] |= GetLayerBit("Projectile");
        layerMasks[layerBit] |= GetLayerBit("Wall");
        layerBit = GetLayerBit("Projectile");
        layerBit = GetLayerBit("Wall");
        layerMasks[layerBit] |= GetLayerBit("Player");
        layerMasks[layerBit] |= GetLayerBit("Enemy");
        layerMasks[layerBit] |= GetLayerBit("Projectile");
        layerMasks[layerBit] |= GetLayerBit("Wall");
        return;
    }

    json j = json::parse(settings);

    for (std::string layer : j["layers"])
    {
        RegisterLayer(layer);
    }

    for (auto& [layerBit, layerName] : layerNames)
    {
        if (j.contains(layerName + "mask"))
        {
            for (std::string mask : j[layerName + "mask"])
            {
                layerMasks[layerBit] |= GetLayerBit(mask);
            }
        }
    }

    std::cout << "Collision settings loaded c:\n";

}

void CollisionLayerManager::SaveCollisionSettings() {

    json j;

    std::vector<std::string> collisionLayerList{};
    for (auto& [layerBit, layerName] : layerNames)
    {
        collisionLayerList.push_back(layerName);
    }

    j["layers"] = collisionLayerList;

    for (std::string& name : collisionLayerList) {
        std::vector<std::string> maskNames{};
        uint32_t maskBit = GetLayerMask(GetLayerBit(name));
        for (uint32_t i = 0; i < 32; ++i) {
            uint32_t bit = 1 << i;
            if (maskBit & bit) {
                std::string layerName = CollisionLayerManager::Get().GetLayerName(bit);
                if (!layerName.empty()) {
                    maskNames.push_back(layerName);
                }
            }
        }

        j[name + "mask"] = maskNames;
    }

    FileIO::WriteFile(FilePath::COLLISIONSETTINGS_FILE, j.dump());

}

void CollisionLayerManager::SetLayerMask(uint32_t layerBit, uint32_t maskBit) {
    auto it = layerNames.find(layerBit);
    if (it == layerNames.end())
    {
        std::cerr << "Failed to set layer mask, Collision layer bit " << layerBit << " not registered!\n";
        return;
    }

    layerMasks[layerBit] = maskBit;
}

uint32_t CollisionLayerManager::GetLayerMask(uint32_t layerBit) const {
    auto it = layerMasks.find(layerBit);
    if (it == layerMasks.end())
    {
        std::cerr << "Failed to get layer mask, Collision layer bit " << layerBit << " not registered!\n";
        return 0;
    }

    return it->second;
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
    // If either entity has no layer set (0), don't collide
    // This prevents entities without proper collision setup from affecting the system
    if (layerA == 0 || layerB == 0) {
        return false;
    }

    // Both have layers set, use mask filtering
    // A collides with B if: (A's mask contains B's layer) AND (B's mask contains A's layer)
    // This means BOTH entities must want to collide for collision to happen
    bool result = (maskA & layerB) && (maskB & layerA);

    // Debug: Uncomment to see collision filtering decisions
    // static int debugCounter = 0;
    // if (++debugCounter % 100 == 0) {  // Print every 100th check to avoid spam
    //     std::cout << "[Collision] layerA:0x" << std::hex << layerA << " maskA:0x" << maskA
    //               << " vs layerB:0x" << layerB << " maskB:0x" << maskB
    //               << " -> " << (result ? "COLLIDE" : "SKIP") << std::dec << std::endl;
    // }

    return result;
}

bool CollisionLayerManager::ShouldCollide(uint32_t layerA, uint32_t layerB) const {
    // If either entity has no layer set (0), don't collide
    // This prevents entities without proper collision setup from affecting the system
    if (layerA == 0 || layerB == 0) {
        return false;
    }

    // Both have layers set, use mask filtering
    // A collides with B if: (A's mask contains B's layer) AND (B's mask contains A's layer)
    // This means BOTH entities must want to collide for collision to happen
    uint32_t maskA = GetLayerMask(layerA);
    uint32_t maskB = GetLayerMask(layerB);

    return (maskA & layerB) && (maskB & layerA);

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
    // Safety check: both entities must have all required components
    if (!m_registry.Has_Component<Physics::PhysicsComponent>(a) ||
        !m_registry.Has_Component<Physics::PhysicsComponent>(b)) {
        return;
    }

    auto& transA = m_registry.Get_Component<TransformComponent>(a);
    auto& physA = m_registry.Get_Component<Physics::PhysicsComponent>(a);
    auto& colliderA = m_registry.Get_Component<ColliderComponent>(a);
    auto& transB = m_registry.Get_Component<TransformComponent>(b);
    auto& physB = m_registry.Get_Component<Physics::PhysicsComponent>(b);
    auto& colliderB = m_registry.Get_Component<ColliderComponent>(b);

    const AABB& aabbA = colliderA.Get_AABB();
    const AABB& aabbB = colliderB.Get_AABB();

    // Calculate collision normal
    Vec2 diff = RG_Subtract(transB.pos, transA.pos);
    float overlapX = (aabbA.dim.x + aabbB.dim.x) / 2.0f - RG_Abs(diff.x);
    float overlapY = (aabbA.dim.y + aabbB.dim.y) / 2.0f - RG_Abs(diff.y);

    if (overlapX <= 0 || overlapY <= 0) {
        return;
    }

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
        colliderA.UpdateHitboxes(transA.pos);
        colliderB.UpdateHitboxes(transB.pos);
    }
    else if (!aStatic) {
        Vec2 correction = RG_Scale(normal, -correctionAmount);
        transA.pos = RG_Add(transA.pos, correction);
        colliderA.UpdateHitboxes(transA.pos);
    }
    else if (!bStatic) {
        Vec2 correction = RG_Scale(normal, correctionAmount);
        transB.pos = RG_Add(transB.pos, correction);
        colliderB.UpdateHitboxes(transB.pos);
    }

    // Calculate relative velocity
    Vec2 velA = physA.Get_Vel();
    Vec2 velB = physB.Get_Vel();
    Vec2 relativeVel = RG_Subtract(velA, velB);
    float velAlongNormal = RG_DotProduct(relativeVel, normal);

    // Don't resolve if separating
    if (velAlongNormal > -0.1f) {
        return;
    }

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
void CollisionManager::CollisionActions(Entity a, Entity b, bool intersected, bool isTrigger)
{
    if (!intersected) return;

    // Always generate collision event
    AddEvent(a, b);

    // If it's a trigger, publish event and skip physics response
    if (isTrigger) {
        // Get trigger event names from colliders
        if (m_registry.Has_Component<ColliderComponent>(a) && m_registry.Has_Component<ColliderComponent>(b)) {
            auto& colliderA = m_registry.Get_Component<ColliderComponent>(a);
            auto& colliderB = m_registry.Get_Component<ColliderComponent>(b);

            // Publish event if either collider has a triggerEvent set
            if (!colliderA.triggerEvent.empty()) {
                TriggerEvent event;
                event.a = a;
                event.b = b;
                event.eventName = colliderA.triggerEvent;
                EventBus::Get().PushEvent<TriggerEvent>(event);
                // Remove collider from trigger entity so it only fires once
                m_registry.Remove_Component<ColliderComponent>(a);
            }
            if (!colliderB.triggerEvent.empty()) {
                TriggerEvent event;
                event.a = a;
                event.b = b;
                event.eventName = colliderB.triggerEvent;
                EventBus::Get().PushEvent<TriggerEvent>(event);
                // Remove collider from trigger entity so it only fires once
                m_registry.Remove_Component<ColliderComponent>(b);
            }
        }
        return;
    }

    // Apply physics-based collision response
    ActionResolve_ImpulseBased(a, b);
}

void CollisionManager::CheckCollisions() {
    // Clear collision markers from previous frame FIRST (before clearing grid)
    spatialGrid.ClearCollisionMarkers();

    auto& colliders = m_registry.Get_ComponentMap<ColliderComponent>();

    // Clear and rebuild spatial grid
    spatialGrid.Clear();

    // BROAD PHASE: Build spatial grid
    for (auto& [entity, collider] : colliders) {
        if (!collider.IsActive() || !m_registry.Has_Component<TransformComponent>(entity)) {
            continue;
        }

        auto& trans = m_registry.Get_Component<TransformComponent>(entity);
        collider.UpdateHitboxes(trans.pos);

        // Insert into spatial grid
        spatialGrid.Insert(entity, collider.Get_AABB());
    }

    // NARROW PHASE: Get potential pairs from spatial grid and check them
    auto potentialPairs = spatialGrid.GetAllPotentialPairs();

    for (const auto& [a, b] : potentialPairs) {
        // Skip self-collision
        if (a == b) {
            continue;
        }

        // Validate both entities still exist and have required components
        if (!m_registry.Has_Component<ColliderComponent>(a) ||
            !m_registry.Has_Component<ColliderComponent>(b)) {
            continue;
        }

        auto& colliderA = m_registry.Get_Component<ColliderComponent>(a);
        auto& colliderB = m_registry.Get_Component<ColliderComponent>(b);

        if (!colliderA.IsActive() || !colliderB.IsActive()) {
            continue;
        }

        // COLLISION LAYER FILTERING
        uint32_t layerA = colliderA.GetLayer();
        //uint32_t maskA = colliderA.GetMask();
        uint32_t layerB = colliderB.GetLayer();
        //uint32_t maskB = colliderB.GetMask();

        // Check if layers should collide using bitmask
        // A should collide with B if: (A's mask & B's layer) OR (B's mask & A's layer)
        //if (!CollisionLayerManager::Get().ShouldCollide(layerA, maskA, layerB, maskB)) {
        //    static int filterCount = 0;
        //    if (++filterCount % 60 == 0) {
        //        std::cout << "[Collision] FILTERED: Entity " << a << " (layer=0x" << std::hex << layerA
        //                  << " mask=0x" << maskA << ") vs Entity " << b << " (layer=0x" << layerB
        //                  << " mask=0x" << maskB << ")" << std::dec << std::endl;
        //    }
        //    continue;
        //}
        if (!CollisionLayerManager::Get().ShouldCollide(layerA, layerB)) {
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

        //if (m_registry.Has_Component<ShapeComponent>(a)) {
        //    auto& shapeA = m_registry.Get_Component<ShapeComponent>(a);
        //    aIsCircle = (shapeA.shape == SHAPE_CIRCLE);
        //}
        //if (m_registry.Has_Component<ShapeComponent>(b)) {
        //    auto& shapeB = m_registry.Get_Component<ShapeComponent>(b);
        //    bIsCircle = (shapeB.shape == SHAPE_CIRCLE);
        //}

        // NARROW PHASE: Precise collision detection
        bool hit = false;

        if (!aIsCircle && !bIsCircle) {
            // Rect-Rect
            hit = CollisionIntersection_RectRect_Static(colliderA.Get_AABB(), colliderB.Get_AABB());
        }
        else if (aIsCircle && bIsCircle) {
            // Circle-Circle
            hit = CollisionIntersection_CC_Static(colliderA.Get_Circle(), colliderB.Get_Circle());
        }
        else {
            // Circle-Rect
            hit = aIsCircle ?
                CollisionIntersection_CA_Static(colliderA.Get_Circle(), colliderB.Get_AABB()) :
                CollisionIntersection_CA_Static(colliderB.Get_Circle(), colliderA.Get_AABB());
        }

        if (hit) {
            // Mark cells where collision occurred (intersection region only)
            spatialGrid.MarkCollisionBetween(colliderA.Get_AABB(), colliderB.Get_AABB());

            // Check if either entity is a trigger
            bool isTrigger = colliderA.IsTrigger() || colliderB.IsTrigger();

            CollisionActions(a, b, true, isTrigger);
        }
    }
}

SpatialGrid* CollisionManager::GetSpatialGrid() {
    return &spatialGrid;
}

// ============================================================================
// ColliderComponent string-based layer methods
// ============================================================================

void ColliderComponent::SetLayer(const std::string& name) {
    //layerName = name;
    layerBit = CollisionLayerManager::Get().GetLayerBit(name);
}

//void ColliderComponent::SetMask(const std::vector<std::string>& names) {
//    //maskNames = names;
//    maskBit = 0;
//    for (const auto& name : names) {
//        maskBit |= CollisionLayerManager::Get().GetLayerBit(name);
//    }
//}
//
//void ColliderComponent::SetMask(uint32_t mask) {
//    maskBit = mask;
//    //maskNames.clear();
//
//    // Convert bits back to names by checking each registered layer
//    // We need to iterate through all possible bit positions (0-31)
//    //for (uint32_t i = 0; i < 32; ++i) {
//    //    uint32_t bit = 1 << i;
//    //    if (mask & bit) {
//    //        std::string name = CollisionLayerManager::Get().GetLayerName(bit);
//    //        if (!name.empty()) {
//    //            maskNames.push_back(name);
//    //        }
//    //    }
//    //}
//}

std::string ColliderComponent::Serialize() const {
    std::string layerName = CollisionLayerManager::Get().GetLayerName(layerBit);
    //std::vector<std::string> maskNames;

    //for (uint32_t i = 0; i < 32; ++i) {
    //    uint32_t bit = 1 << i;
    //    if (maskBit & bit) {
    //        std::string name = CollisionLayerManager::Get().GetLayerName(bit);
    //        if (!name.empty()) {
    //            maskNames.push_back(name);
    //        }
    //    }
    //}

    json j = {
        {"aabb_minX", aabbhitbox.min.x}, {"aabb_minY", aabbhitbox.min.y},
        {"aabb_maxX", aabbhitbox.max.x}, {"aabb_maxY", aabbhitbox.max.y},
        {"aabb_dimX", aabbhitbox.dim.x}, {"aabb_dimY", aabbhitbox.dim.y},
        {"circle_posX", circlehitbox.pos.x}, {"circle_posY", circlehitbox.pos.y},
        {"circle_rad", circlehitbox.rad},
        {"layer", layerName},
        //{"mask", maskNames},
        {"isTrigger", isTrigger}, {"isActive", isActive},
        {"triggerEvent", triggerEvent}
    };
    return j.dump();
}

ColliderComponent ColliderComponent::DeSerialize(Entity eID, const std::string& data) {
    (void)eID;
    if (data.empty()) return ColliderComponent();

    json j = json::parse(data);

    AABB aabb;
    aabb.min = { j.value("aabb_minX", 0.f), j.value("aabb_minY", 0.f) };
    aabb.max = { j.value("aabb_maxX", 0.f), j.value("aabb_maxY", 0.f) };
    aabb.dim = { j.value("aabb_dimX", 0.f), j.value("aabb_dimY", 0.f) };

    Circle circle;
    circle.pos = { j.value("circle_posX", 0.f), j.value("circle_posY", 0.f) };
    circle.rad = j.value("circle_rad", 0.f);

    ColliderComponent comp(aabb, circle, j.value("isActive", true));
    comp.isTrigger = j.value("isTrigger", false);
    comp.triggerEvent = j.value("triggerEvent", "");

    // Load layer/mask names and convert to bits
    if (j.contains("layer")) {
        comp.SetLayer(j["layer"].get<std::string>());
    }
    //if (j.contains("mask")) {
    //    comp.SetMask(j["mask"].get<std::vector<std::string>>());
    //}

    return comp;
}

// ============================================================================
// Setup collision layers based on TagComponent
// ============================================================================

//void CollisionManager::SetupCollisionLayers() {
//    Registry& reg = Registry::Get();
//    EntityManager& em = EntityManager::get();
//
//    int setupCount = 0;
//
//    // Setup collision layers for all entities with both TagComponent and ColliderComponent
//    // Note: Migration from old Physics format is handled in PhysicsComponent::DeSerialize
//    for (Entity entity = 0; entity < em.maxEntities(); ++entity) {
//        if (!em.isAlive(entity)) continue;
//        if (!reg.Has_Component<TagComponent>(entity)) continue;
//        if (!reg.Has_Component<ColliderComponent>(entity)) continue;
//
//        auto& tag = reg.Get_Component<TagComponent>(entity);
//        auto& collider = reg.Get_Component<ColliderComponent>(entity);
//
//        // Preserve custom collision values that were set in the editor
//        // But ALWAYS fix isTrigger and isStatic to ensure collision works
//        bool hasCustomLayers = (collider.GetLayer() != 0 && collider.GetMask() != 0);
//
//        if (hasCustomLayers) {
//            // Entity has custom layer/mask set in editor, preserve them
//            // But still fix trigger and static flags
//            collider.SetTrigger(false);
//            if (reg.Has_Component<Physics::PhysicsComponent>(entity)) {
//                auto& phys = reg.Get_Component<Physics::PhysicsComponent>(entity);
//                if (tag.tag == T_PLAYER || tag.tag == T_ENEMY) {
//                    phys.SetStatic(false);  // Players and enemies must move
//                }
//            }
//            continue;  // Skip resetting layer/mask
//        }
//
//        setupCount++;
//
//        // Set layer and mask based on tag (only for entities with no layer/mask set)
//        switch (tag.tag) {
//            case T_PLAYER:
//                collider.SetLayer("Player");
//                collider.SetMask({"Enemy", "Projectile", "Wall"});
//                collider.SetTrigger(false);  // Player needs physics response
//                // Also fix physics component if it exists
//                if (reg.Has_Component<Physics::PhysicsComponent>(entity)) {
//                    auto& phys = reg.Get_Component<Physics::PhysicsComponent>(entity);
//                    phys.SetStatic(false);  // Player must NOT be static
//                }
//                break;
//
//            case T_ENEMY:
//                collider.SetLayer("Enemy");
//                collider.SetMask({"Player", "Projectile", "Wall"});
//                collider.SetTrigger(false);  // Enemy needs physics response
//                // Also fix physics component if it exists
//                if (reg.Has_Component<Physics::PhysicsComponent>(entity)) {
//                    auto& phys = reg.Get_Component<Physics::PhysicsComponent>(entity);
//                    phys.SetStatic(false);  // Enemy must NOT be static
//                }
//                break;
//
//            case T_PROJECTILE:
//                collider.SetLayer("Projectile");
//                // I think we can change this later on so that we have player projectiles and enemy projectiles?
//                collider.SetMask({"Player", "Enemy", "Wall"});
//                collider.SetTrigger(false);  // Projectile needs physics response
//                break;
//
//            case T_WALL:
//                collider.SetLayer("Wall");
//                collider.SetMask({"Player", "Enemy", "Projectile"});
//                collider.SetTrigger(false);  // Wall needs physics response
//                break;
//
//            case T_OBJECT:
//                // Regular objects collide with everything
//                collider.SetLayer("Player");
//                collider.SetMask({"Player", "Enemy", "Projectile", "Wall"});
//                break;
//
//            default:
//                // Collide with everything by default
//                collider.SetLayer("Player");
//                collider.SetMask({"Player", "Enemy", "Projectile", "Wall"});
//                break;
//        }
//    }
//
//    std::cout << "[Collision] Setup layers for " << setupCount << " entities" << std::endl;
//
//    // Debug: Print all entity collision layers after setup
//    std::cout << "\n[Collision] Final entity collision layers:\n";
//    for (Entity entity = 0; entity < em.maxEntities(); ++entity) {
//        if (!em.isAlive(entity)) continue;
//        if (!reg.Has_Component<ColliderComponent>(entity)) continue;
//
//        auto& collider = reg.Get_Component<ColliderComponent>(entity);
//        std::string tagName = "UNKNOWN";
//        if (reg.Has_Component<TagComponent>(entity)) {
//            auto& tag = reg.Get_Component<TagComponent>(entity);
//            switch(tag.tag) {
//                case T_PLAYER: tagName = "PLAYER"; break;
//                case T_ENEMY: tagName = "ENEMY"; break;
//                case T_WALL: tagName = "WALL"; break;
//                case T_PROJECTILE: tagName = "PROJECTILE"; break;
//                case T_OBJECT: tagName = "OBJECT"; break;
//                default: tagName = "OTHER"; break;
//            }
//        }
//
//        std::cout << "  Entity " << entity << " (" << tagName << "): layer=0x"
//                  << std::hex << collider.GetLayer() << " mask=0x" << collider.GetMask()
//                  << std::dec << " active=" << (collider.IsActive() ? "YES" : "NO")
//                  << " trigger=" << (collider.IsTrigger() ? "YES" : "NO") << "\n";
//    }
//    std::cout << std::endl;
//}
