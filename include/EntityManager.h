#pragma once

#include "Entity.h"

#include "Component.h"


// DO NOT CALL THIS DIRECTLY, GO THROUGH REGISTRY!!!
class EntityManager
{
private:
	Entity next_entID = 0;
	// std::vector<ComponentMask> ent_masks;
	// std::vector<Entity>		entlist;	// List of all entity IDs
	std::vector<Entity>		freeID;		// Recycle old IDs to prevent next_entID from increasing infinitely
	std::vector<bool>		alive;		// Track which ID is alive, vector grows as new entities are added

private:
	// Private ctor for singleton
	EntityManager()									= default;

	// Delete copy/move to ensure only one instance
	EntityManager(const EntityManager&)				= delete;
	EntityManager& operator=(const EntityManager&)	= delete;

	// do not need custom dtor due standard types
	~EntityManager()								= default;


public:

	// for singleton pattern, to ensure there is only ONE of each entity manager
	// use example: Entity blabla = EntityManager::get().Create_Entity();
	static EntityManager& get();

	Entity createEntity();
	void destroyEntity(Entity e);

	bool isAlive(Entity e) const;
	size_t maxEntities() const { return alive.size(); }


};