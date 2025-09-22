// component manager class

#pragma once

#include "Component.h"
#include "Entity.h"
#include <unordered_map>

template<typename Component>
class ComponentManager {
public:
	void		addComponent(Entity e, Component const& comp);
	void		removeComponent(Entity e);
	Component&	getComponent(Entity e);

private:
	std::unordered_map<Entity, Component> components;
};