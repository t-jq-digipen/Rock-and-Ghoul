
#include "ComponentManager.h"
#include <iostream>
#include <string>

template<typename Component>
void ComponentManager<Component>::addComponent(Entity eID, const Component& comp)
{
	// check if entity already has component
	if (components.find(eID) != components.end())
	{
		throw std::runtime_error("Entity already has component");
	}
	else // entity does not have this component
	{
		// create a new component element with eID key
		components[eID] = comp;
	}
}

template<typename Component>
void ComponentManager<Component>::removeComponent(Entity eID)
{
	// remove this component from the map
	components.erase(eID);
}

template<typename Component>
bool ComponentManager<Component>::hasComponent(Entity eID)
{
	return (components.find(eID) != components.end());
}

template<typename Component>
Component& ComponentManager<Component>::getComponent(Entity eID)
{
	if (components.find(eID) != components.end())
	{
		return components[eID];
	}
	throw std::runtime_error("Entity " + std::to_string(eID) + " does NOT have component\n");
}