/* ========================================================
 * @file		EntityManager.cpp
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @date		01 OCT 2025
 *
 * @brief	Entity Manager for ECS
 *
 * Singleton exists within Registry.
 *
 * This file implements the functionality of the Entity Manager
 * declared in EntityManager.h.
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

// DO NOT AMEND THIS FILE
#include "EntityManager.h"
#include <iostream>
#include <string>

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


	alive[e] = true;	// set new entity to alive ^^
	return e;
}

// Destroy entity
void EntityManager::destroyEntity(Entity eID)
{
	// Check if out of scope
	if (eID < 0 || eID >= alive.size())
	{	throw std::runtime_error("Sumtingwong ya this entity" + std::to_string(eID) +  "cannot be destroy! Why you want to destroy!?\n");	}

	// Check if entity is already dead
	if (!alive[eID]) { return; }

	// Set alive flag to false, and add ID to list of free IDs
	alive[eID] = false;
	freeID.push_back(eID);

	// NOTE: COMPONENTS removed in REGISTRY
}

// Alive check!
bool EntityManager::isAlive(Entity eID) const
{
	// Check if out of scope
	if (eID < 0 || eID >= alive.size())
	{	throw std::runtime_error("Sumtingwong ya this entity cannot be alive! check ur code!\n");	}


	// return if alive :) life is good
	return alive[eID];
}

