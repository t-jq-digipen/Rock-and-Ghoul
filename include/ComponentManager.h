// component manager class

#pragma once

#include "Component.h"
#include "EntityManager.h"
#include <unordered_map>
#include <iostream>
#include <string>


// template for component manager for each component
// each component to have its own manager e.g ComponentManager<TransformComponent>
template<typename Component>
class ComponentManager {

private:
	// Map of ENTITY ID and COMPONENT
	std::unordered_map<Entity, Component> components;

public:

	// Check if entity has component
	bool		hasComponent(Entity eID) { return (components.find(eID) != components.end()); }

	// Add a component to an entity
	void		addComponent(Entity eID, Component const& comp)
	{
		// check if entity already has component
		if (hasComponent(eID))
		{
			throw std::runtime_error("Entity already has component");
		}
		else // entity does not have this component
		{
			// create a new component element with eID key
			components[eID] = comp;
		}
	}

	// Remove a component from an entity
	void		removeComponent(Entity eID) { components.erase(eID); }

	// Return reference to component struct
	Component&	getComponent(Entity eID)
	{
		auto it = components.find(eID);
		if (it != components.end())
		{
			return it->second;
		}
		throw std::runtime_error("Entity " + std::to_string(eID) + " does NOT have component\n");
	}

};
