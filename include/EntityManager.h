/* ========================================================
 * @file		EntityManager.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @date		01 OCT 2025
 *
 * @brief	Entity Manager for ECS
 *
 * Singleton exists within Registry.
 * 
 * This EntityManager is responsible for creating, destroying,
 * and tracking entities within the ECS. Each entity is
 * represented with a unique ID, and the EntityManager
 * recycles old IDs to prevent unbounded growth.
 * 
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

// DO NOT AMEND THIS FILE
#pragma once

#include "Entity.h"
#include <unordered_map>

// #include "Component.h"


 /**
  * @class EntityManager
  * @brief Singleton class that manages entity lifecycle in ECS.
  *
  * EntityManager generates unique entity IDs, recycles old IDs, and tracks
  * whether entities are alive or destroyed. It ensures that the system does
  * not run out of IDs by reusing previously freed ones.
  *
  * This class should be accessed only via `EntityManager::get()`.
  */
class EntityManager
{
private:
	Entity next_entID = 0;			// Next available entity ID

	std::vector<Entity>	freeID;		// Recycle old IDs to prevent next_entID from increasing infinitely
	std::vector<bool>	alive;		// Track which ID is alive, vector grows as new entities are added

	std::unordered_map<Entity, std::string> entityNames;	// Map to store entity names
	std::unordered_map<std::string, Entity> nameToEntity;
private:
	// Private ctor for singleton
	EntityManager()									= default;

	// Delete copy/move to ensure only one instance
	EntityManager(const EntityManager&)				= delete;
	EntityManager& operator=(const EntityManager&)	= delete;

	// do not need custom dtor due standard types
	~EntityManager()								= default;


public:

	/**
	 * @brief Access the singleton instance of EntityManager.
	 * @return A reference to the single EntityManager instance.
	 *
	 * Example:
	 * EntityManager& manager = EntityManager::get();
	 */
	static EntityManager& get();

	/**
	 * @brief Create a new entity.
	 *
	 * If there are recycled IDs available, reuse one; otherwise allocate a new one.
	 * The entity is automatically marked as alive.
	 *
	 * @return The newly created entity ID.
	 */
	Entity createEntity();
	/**
	 * @brief Destroy an existing entity.
	 *
	 * Marks the entity as dead and adds its ID to the free list for recycling.
	 * Does not automatically remove attached components; that should be handled
	 * by the ECS registry.
	 *
	 * @param e The entity ID to destroy.
	 * @throws std::runtime_error if the entity ID is out of range.
	 */
	void destroyEntity(Entity e);


	/**
	 * @brief Check if an entity is alive.
	 *
	 * @param e The entity ID to check.
	 * @return true if the entity exists and is alive, false otherwise.
	 * @throws std::runtime_error if the entity ID is out of range.
	 */
	bool isAlive(Entity e) const;

	/**
	 * @brief Get the maximum number of entities tracked so far.
	 *
	 * This corresponds to the size of the alive vector and indicates
	 * how many IDs have been allocated at least once.
	 *
	 * @return The number of entities currently tracked.
	 */
	size_t maxEntities() const { return alive.size(); }

	/**
	 * @brief Get the map of entity names.
	 *
	 * @return Reference to unordered map of entity IDs to their names
	 */
	std::unordered_map<Entity, std::string>& GetEntityNameMap();

	/**
	 * @brief Sets the name of an entity.
	 * 
	 * @param eID The entity ID.
	 * @param name The name to be assigned to the entity.
	 */
	void SetEntityName(Entity eID, const std::string& name);

	/**
	 * @brief Sets the name of an entity.
	 * 
	 * @param eID The entity ID.
	 * @return The name of the entity.
	 */
	std::string GetEntityName(Entity eID);

	Entity GetEntityFromName(const std::string& name);
	void EnsureEntityID(Entity id);
};