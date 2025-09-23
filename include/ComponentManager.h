// component manager class

#pragma once

#include "Component.h"
#include "EntityManager.h"
#include <unordered_map>


// template for component manager for each component
// each component to have its own manager e.g ComponentManager<TransformComponent>
template<typename Component>
class ComponentManager {
public:
	void		addComponent(Entity eID, Component const& comp);	// Add a component to an entity
	void		removeComponent(Entity eID);						// Remove a component from an entity
	bool		hasComponent(Entity eID);							// Check if entity has component
	Component&	getComponent(Entity eID);							// Return reference to component struct
	Entity&		getEntityID(Component& cID);						// Return entity ID from component

private:
	// Map of ENTITY ID and COMPONENT
	std::unordered_map<Entity, Component> components;
};
