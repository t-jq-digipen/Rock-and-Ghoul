#include "EntityManager.h"
#include <iostream>

// Singleton access
EntityManager& EntityManager::get()
{
	static EntityManager instance;
	return instance;
}

// Create new entity
Entity EntityManager::createEntity()
{
	Entity e {};
	// If there are free IDs, recycle from the back of the list
	if (!freeID.empty())
	{ 
		e = freeID.back(); 
		freeID.pop_back();
	}
	else	// there are no more free IDs, use next ID!
	{	
		e = next_entID++;
	}

	// Ensure alive vector is large enough
	if (alive.size() < next_entID)
	{	alive.resize(next_entID, false);	}


	alive[e] = true;		// set new entity to alive ^^
	return e;
}

// Destroy entity
void EntityManager::destroyEntity(Entity e)
{
	// Check if out of scope
	if (e < 0 || e >= alive.size())
	{	std::cerr << "Sumtingwong ya this entity cannot be destroy! Why you want to destroy!?\n";	}

	// Check if entity is already dead
	if (!alive[e]) { return; }

	// Remove components
	// TO DO (PSEUDO)
	/*
	for (component:componentslist)
	{
		if (componentmanager<component>.hasComponent(eID)
		{
			componentmanager<component>.removeComponent(eID);
		}
	}
	*/

	// Set alive flag to false, and add ID to list of free IDs
	alive[e] = false;
	freeID.push_back(e);
}

// Alive check!
bool EntityManager::isAlive(Entity e) const
{
	// Check if out of scope
	if (e < 0 || e >= alive.size())
	{	std::cerr << "Sumtingwong ya this entity cannot be alive! check ur code!\n";	}


	// return if alive :) life is good
	return alive[e];
}