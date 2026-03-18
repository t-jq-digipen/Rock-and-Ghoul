/* ========================================================
 * @file        CollisionManager.h
 * @author      Yasmin Tazi        | yasmin.t@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        28 NOV 2025
 *
 * @brief
 *
 * This header defines the CollisionManager class, which is responsible
 * for detecting overlaps between entities based on their Collider
 * components, storing collision events, and resolving them.
 *
 * Also defines:
 *   - CollisionLayerManager: manages collision layers and masks
 *   - CollisionEvent: stores collision pairs for processing
 *   - TriggerEvent: event published when trigger colliders overlap
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "Component.h"
#include "Collision.h"
#include "Registry.h"
#include "EventBus.h"
#include <vector>
#include <iostream>

struct CollisionEvent {
    Entity a;
    Entity b;
};

struct TriggerEvent : public IEvent {
    Entity trigger;      // The "trigger" entity
    Entity other;        // The "victim" entity
    std::string eventName; // triggerEvent
};

// convert a tag enum to a readable string
//constexpr const char* TagToString(C_TagID tag) {
//    switch (tag) {
//    case T_PLAYER:     return "Player";
//    case T_ENEMY:      return "Enemy";
//    case T_PROJECTILE: return "Projectile";
//    default:           return "Unknown";
//    }
//}

class CollisionLayerManager {
private:
    std::unordered_map<std::string, uint32_t> layerBits;
    std::unordered_map<uint32_t, std::string> layerNames;
    std::unordered_map<uint32_t, uint32_t> layerMasks;
    uint32_t nextBit = 0;

    CollisionLayerManager() {}
    CollisionLayerManager(const CollisionLayerManager&) = delete;
public:
    static CollisionLayerManager& Get() {
        static CollisionLayerManager instance;
        return instance;
    }

    uint32_t RegisterLayer(const std::string& name);

    void LoadCollisionSettings();

    void SaveCollisionSettings();

    void SetLayerMask(uint32_t layerBit, uint32_t maskBit);

    uint32_t GetLayerMask(uint32_t layerBit) const;

    uint32_t GetLayerBit(const std::string& name) const;

    std::string GetLayerName(uint32_t bit) const;

    const std::unordered_map<uint32_t, std::string>& GetLayerNameMap() const { return layerNames; }

    bool ShouldCollide(uint32_t layerA, uint32_t maskA, uint32_t layerB, uint32_t maskB) const;

    bool ShouldCollide(uint32_t layerA, uint32_t layerB) const;

    void RemoveLayer(const std::string& name);

    void Clear();
};


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
    //void ActionStop(Entity a, Entity b);

    //void ActionStop_Static(Entity dynamic, Entity staticObj);

    void CollisionActions(Entity a, Entity b, bool intersected, bool isTrigger = false);

    /**
     * @brief Resolves collision using impulse-based response (MEETS RUBRIC).
     * @param a The first colliding entity.
     * @param b The second colliding entity.
     */
    void ActionResolve_ImpulseBased(Entity a, Entity b);

    /**
     * @brief Get spatial grid statistics for debugging/profiling
     * @return Pointer to the spatial grid, or nullptr if not available
     */
    static class SpatialGrid* GetSpatialGrid();

    /**
     * @brief Setup collision layers for all entities based on their TagComponent
     *
     * This helper function iterates through all entities with TagComponents
     * and automatically assigns collision layers and masks based on their tag type.
     * Call this after loading entities (e.g., in Level1::Load()).
     */
    //static void SetupCollisionLayers();
};
