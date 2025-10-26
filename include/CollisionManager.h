/* ========================================================
 * @file        CollisionManager.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * This header defines the CollisionManager class, which is responsible
 * for detecting overlaps between entities based on their Physics and
 * Shape components, storing collision events, and resolving them.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "Component.h"
#include "Collision.h"
#include "Registry.h"
#include <vector>
#include <iostream>

struct CollisionEvent {
    Entity a;
    Entity b;
};
// convert a tag enum to a readable string
constexpr const char* TagToString(C_TagID tag) {
    switch (tag) {
    case T_PLAYER:     return "Player";
    case T_ENEMY:      return "Enemy";
    case T_PROJECTILE: return "Projectile";
    default:           return "Unknown";
    }
}

/**
 * @class CollisionManager
 * @brief Manages detection and resolution of collisions between entities.
 */

class CollisionManager {
private:
    //stores a list of all collision events found during one frame
    std::vector<CollisionEvent> events;

    // Reference to registry
    Registry& m_registry;

public:
    // Default ctor to initialize registry reference
    CollisionManager() : m_registry(Registry::Get()) {}
    /**
     * @brief Clears all collision events for the current frame.
     */
    void Clear();
    /**
     * @brief Adds a collision event between two entities.
     * @param a The first entity.
     * @param b The second entity.
     */
    void AddEvent(Entity a, Entity b);
    /**
     * @brief Processes all stored collision events.
     */
    void ProcessEvents();
    /**
     * @brief Performs collision checks between all active entities.
     */
    void CheckCollisions();
    /**
     * @brief Resolves a collision by rolling entities back and zeroing velocity.
     * @param a The first colliding entity.
     * @param b The second colliding entity.
     */
    void ActionStop(Entity a, Entity b);

    void ActionStop_Static(Entity dynamic, Entity staticObj);

    void CollisionActions(Entity a, Entity b, bool intersected);

    /**
     * @brief Resolves collision using impulse-based response (MEETS RUBRIC).
     * @param a The first colliding entity.
     * @param b The second colliding entity.
     */
    void ActionResolve_ImpulseBased(Entity a, Entity b);
};
