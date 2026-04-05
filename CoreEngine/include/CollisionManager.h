/* ========================================================
 * @file        CollisionManager.h
 * @author      Yasmin Tazi        | yasmin.t@digipen.edu 70%
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu 30%
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

/**
 * @struct CollisionEvent
 * @brief Stores a pair of entities that collided during the current frame.
 */
struct CollisionEvent {
    Entity a; //First entity in the collision pair.
    Entity b; //Second entity in the collision pair.
};

/**
 * @struct TriggerEvent
 * @brief Event published through the EventBus when a trigger collider overlaps another collider.
 */
struct TriggerEvent : public IEvent {
    Entity trigger;         //The entity whose collider is marked as a trigger.
    Entity other;           // The other entity that entered the trigger volume.
    std::string eventName;  // Name of the event (e.g. "triggerEvent").
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

/**
 * @class CollisionLayerManager
 * @brief Singleton that manages named collision layers and their interaction masks.
 *
 * Each layer is represented as a single bit in a 32-bit integer.
 * A layer's mask defines which other layers it can collide with.
 * Layer settings are persisted to and loaded from a JSON config file.
 */
class CollisionLayerManager {
private:
    std::unordered_map<std::string, uint32_t> layerBits;
    std::unordered_map<uint32_t, std::string> layerNames;
    std::unordered_map<uint32_t, uint32_t> layerMasks;
    uint32_t nextBit = 0;

    CollisionLayerManager() {}
    CollisionLayerManager(const CollisionLayerManager&) = delete;
public:
    /**
     * @brief Returns the singleton instance of CollisionLayerManager.
     * @return Reference to the single CollisionLayerManager instance.
     */
    static CollisionLayerManager& Get() {
        static CollisionLayerManager instance;
        return instance;
    }

    /**
     * @brief Registers a new collision layer by name, assigning it the next available bit.
     * @param name for the layer (e.g. "Player", "Enemy").
     * @return The bit value assigned to the new layer, or the existing bit if already registered.
     */
    uint32_t RegisterLayer(const std::string& name);

    /**
     * @brief Loads collision layer definitions and masks from collision settings JSON file.
     */
    void LoadCollisionSettings();

    /**
     * @brief Saves the current collision layer definitions and masks to collision settings JSON file.
     */
    void SaveCollisionSettings();

    /**
     * @brief Sets the collision mask for a given layer bit.
     * @param layerBit The bit representing the layer whose mask is being set.
     * @param maskBit  The new 32-bit mask indicating which layers this layer collides with.
     */
    void SetLayerMask(uint32_t layerBit, uint32_t maskBit);

    /**
     * @brief Returns the collision mask for a given layer bit.
     * @param layerBit The bit representing the layer to query.
     * @return The 32-bit collision mask for that layer, or 0 if not registered.
     */
    uint32_t GetLayerMask(uint32_t layerBit) const;

    /**
     * @brief Looks up the bit value for a named layer.
     * @param name The layer name to look up.
     * @return The bit value for the named layer, or 0 if not found.
     */
    uint32_t GetLayerBit(const std::string& name) const;

    /**
     * @brief Looks up the name of a layer from its bit value.
     * @param bit The bit value to look up.
     * @return The layer name string, or an empty string if not found.
     */
    std::string GetLayerName(uint32_t bit) const;

    /**
     * @brief Returns the full map of bit values to layer names.
     * @return Const reference to the internal bit-to-name map.
     */
    const std::unordered_map<uint32_t, std::string>& GetLayerNameMap() const { return layerNames; }

    /**
     * @brief Checks whether two layers should collide, given explicit masks for both.
     * @param layerA Bit of the first layer.
     * @param maskA  Collision mask of the first layer.
     * @param layerB Bit of the second layer.
     * @param maskB  Collision mask of the second layer.
     */
    bool ShouldCollide(uint32_t layerA, uint32_t maskA, uint32_t layerB, uint32_t maskB) const;

    /**
     * @brief Checks whether two layers should collide using their registered masks.
     * @param layerA Bit of the first layer.
     * @param layerB Bit of the second layer.
     * @return True if the registered masks for both layers indicate mutual collision.
     */
    bool ShouldCollide(uint32_t layerA, uint32_t layerB) const;

    /**
     * @brief Removes a registered layer by name, clearing its bit and mask entries.
     * @param name The name of the layer to remove.
     */
    void RemoveLayer(const std::string& name);

    /**
     * @brief Clears all registered layers and masks, resetting to the initial empty state.
     */
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
    /**
     * @brief Constructs the CollisionManager and binds it to the engine Registry singleton.
     */
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
     * @brief Resolves collision using impulse-based response.
     * @param a The first colliding entity.
     * @param b The second colliding entity.
     */
    void ActionResolve_ImpulseBased(Entity a, Entity b);

    /**
     * @brief Get spatial grid statistics for debugging
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
