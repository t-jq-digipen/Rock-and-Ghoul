/* ========================================================
 * @file		ComponentManager.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @date		27 SEP 2025
 *
 * @brief	Component Manager template for ECS
 * 
 * The Component Manager is responsible for storing, retrieving
 * and removing components of a given type. Each component type
 * has it's own manager.
 * e.g ComponentManager<TransformComponent>
 * These component managers exist in Registry.
 * 
 * Entities must still be managed through the EntityManager!
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

// DO NOT AMEND
#pragma once

#include "Component.h"
#include "EntityManager.h"
#include <unordered_map>
#include <iostream>
#include <string>

// type erased component manager for registry storage
class IComponentManager {
public:
	virtual ~IComponentManager() = default;
	virtual void removeComponent(Entity eID) = 0;

	virtual std::string SaveComponent(Entity eID) = 0;
	virtual void LoadComponent(Entity eID, const std::string& data) = 0;
};

// template for component manager for each component
template<typename Component>
class ComponentManager : public IComponentManager {

private:
	// Map of ENTITY ID and COMPONENT
	std::unordered_map<Entity, Component> components;

public:

	/**
	 * @brief Check if an entity has this component type.
	 * @param eID The entity ID to check.
	 * @return True if the entity has the component, false otherwise.
	 */
	bool		hasComponent(Entity eID) { return (components.find(eID) != components.end()); }

	/**
	 * @brief Add a component to an entity.
	 *
	 * Associates a new component of type `Component` with the given entity.
	 * Throws if the entity already has a component of this type.
	 *
	 * @param eID The entity ID.
	 * @param comp The component instance to add.
	 * @throws std::runtime_error if the entity already has this component.
	 */
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
			std::cout << "Component created\n";
		}
	}

	/**
	 * @brief Remove a component from an entity.
	 * @param eID The entity ID.
	 */
	void		removeComponent(Entity eID)  override { components.erase(eID); }

	/**
	 * @brief Get a reference to an entity's component.
	 *
	 * @param eID The entity ID.
	 * @return Reference to the component associated with the entity.
	 * @throws std::runtime_error if the entity does not have this component.
	 */
	Component&	getComponent(Entity eID)
	{
		auto it = components.find(eID);
		if (it != components.end())
		{
			return it->second;
		}
		throw std::runtime_error("Entity " + std::to_string(eID) + " does NOT have component\n");
	}

	/**
	 * @brief Get all components managed by this manager.
	 * @return Reference to the internal unordered_map of entity IDs to components.
	 */
	std::unordered_map<Entity, Component>& getComponents() { return components; }
	/**
	 * @brief Get all components managed by this manager (const version).
	 * @return Const reference to the internal unordered_map of entity IDs to components.
	 */
	const std::unordered_map<Entity, Component>& getComponents() const { return components; }

	std::string SaveComponent(Entity eID) override
	{
		auto it = components.find(eID);
		if (it != components.end())
		{
			return it->second.Serialize();
		}
		return "";
	}

	void LoadComponent(Entity eID, const std::string& data) override
	{
		Component newComp = Component::DeSerialize(data);
		addComponent(eID, newComp);
	}

};

// Class for serialization of components
//class ComponentSerializer {
//public:
//	// Save function pointer, returns a string
//	using SaveFn = std::function<std::string(void*)>;
//	// Load function pointer, returns void and takes const std::string&
//	using LoadFn = std::function<void(void*, const std::string&)>;
//
//private:
//	ComponentSerializer() {}
//	ComponentSerializer(const ComponentSerializer&) = delete;
//
//	// Map of functions keyed with type_index
//	std::unordered_map<std::type_index, SaveFn> saveFunctions;
//	std::unordered_map<std::type_index, LoadFn> loadFunctions;
//
//public:
//	// Singleton access
//	static ComponentSerializer& Get() {
//		static ComponentSerializer instance;
//		return instance;
//	}
//
//	template<typename Component>
//	void Register() {
//		std::type_index typeIndex(typeid(Component));
//
//		saveFunctions[typeIndex] = 
//	}
//
//
//
//};