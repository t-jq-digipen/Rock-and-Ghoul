/* ========================================================
 * @file		Registry.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu 100%

 * @date		30 OCT 2025
 *
 * @brief	Registry for ECS management
 *
 * Declares interface functions for Entity and Component
 * management of the ECS architecture. Provides functions
 * for creating, removing, and accessing entities and
 * their components, as well as accessors for the
 * component managers.
 *
 * UPDATE 23 OCT 2025 - Tan Jun Quan
 * Registry is being overhauled to a singleton class with
 * the ability to register component types dynamically,
 * allowing for developers to define their own types of
 * components and systems.
 * The engine will still have default components such as
 * transform, sprite, animator, shape, but this now allows
 * developers to add component types by calling the
 * RegisterComponent function, e.g:
 * - Registry::Get().Register_Component<ComponentTypename>();
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

 // IMPORTANT!!! DO NOT AMEND ANYTHING IN THIS FILES WITHOUT ASKING ME!!!!!!!
#pragma once


// Use entity manager
// Attach components to entities
// Track which entity has what components
// Help systems find stuff in entities
#include <typeindex>		// for type index
#include <unordered_map>	// for map
#include <iostream>
#include <stdexcept>
#include "ComponentManager.h"
#include "EntityManager.h"
#include "Component.h"
#include "Physics.h"		// Physics Component
#include "Sprite.h"			// Sprite Component
#include "Animator.h"		// Animator Component
#include "Movement.h"
#include "DataDefs.h"
#include <nlohmann/json.hpp>


// Make registry a singleton class with template list
class Registry {
private:
	Registry() {}
	Registry(const Registry&) = delete;

	// Reference to entity manager
	EntityManager& entityManager = EntityManager::get();

	// Map of component managers
	std::unordered_map<std::type_index, std::unique_ptr<IComponentManager>> componentManagers;
	// Vector of type index in order of registration
	std::vector<std::type_index> componentTypeList;
	// Map of component type names to type index
	std::unordered_map<std::string, std::type_index> componentTypeNames;
	// Map of type index with a list of dependencies
	std::unordered_map<std::type_index, std::vector<std::type_index>> componentDependencies;
	std::unordered_map<std::type_index, std::vector<std::type_index>> componentDependents;

	/**
	 * @brief Registers a new component type with the registry.
	 *
	 * Creates a ComponentManager<Component> instance and records its
	 * runtime type information, name, and optional dependency list.
	 *
	 * @tparam Component The component struct/class type to register.
	 * @param name Human-readable component name (used for serialization & editor).
	 * @param dependencies Optional list of component type names that must exist
	 *                     on an entity before this component may be added.
	 *
	 * @throws std::runtime_error If the component is already registered.
	 * @throws std::runtime_error If any dependency is not registered first.
	 */
	template<typename Component>
	ComponentManager<Component>* Get_Manager() {
		auto it = componentManagers.find(std::type_index(typeid(Component)));
		if (it == componentManagers.end())
		{
			throw std::runtime_error("COMPONENT NOT REGISTERED ???\n");
		}

		return static_cast<ComponentManager<Component>*>(it->second.get());
	}


	// CRASH if never register component type
	void IsComponentRegistered(std::type_index typeID) {
		if (componentManagers.find(typeID) == componentManagers.end())
		{
			throw std::runtime_error("COMPONENT TYPE NOT REGISTERED");
		}
	}

public:
	// REGISTRY
	// ================================================================================
#pragma region REGISTRY
	// Getter for registry instance
	static Registry& Get() {
		static Registry instance;
		return instance;
	}

#pragma endregion REGISTRY
	// ================================================================================

	// COMPONENTS
	// ================================================================================
#pragma region COMPONENTS


	bool HasDependencies(Entity eID, std::type_index typeID) {
		IsComponentRegistered(typeID);

		// Component type has no dependencies!
		if (componentDependencies.find(typeID) == componentDependencies.end())
		{ return true; }

		for (std::type_index dep : componentDependencies[typeID])
		{
			IsComponentRegistered(dep);
			if (!componentManagers[dep]->hasComponent(eID))
			{
				// std::cout << "Entity " << eID << " Dependency type " << Get_ComponentTypeName(dep) << " for " << Get_ComponentTypeName(typeID) << " not found\n";
				return false;
			}
		}
		return true;
	}

	bool HasDependents(Entity eID, std::type_index typeID) {
		IsComponentRegistered(typeID);

		// Component type has no dependents!
		if (componentDependents.find(typeID) == componentDependents.end())
		{ return false; }

		for (std::type_index reliant : componentDependents[typeID])
		{
			IsComponentRegistered(typeID);
			if (componentManagers[reliant]->hasComponent(eID))
			{
				// std::cout << "Entity " << eID << "Component type " << Get_ComponentTypeName(reliant) << " is relying on " << Get_ComponentTypeName(typeID) << '\n';
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief Registers a Component Type to the registry.
	 * @param eID Entity identifier.
	 * @param component Component instance to add.
	 */
	template<typename Component>
	void Register_Component(const std::string& name, const std::vector<std::string>& dependencies = {}) {
		// Assign type index
		std::type_index typeID(typeid(Component));
		// Check if component is already registered
		if ((componentManagers.find(typeID)) != componentManagers.end())
		{
			throw std::runtime_error("COMPONENT " + name + " ALREADY REGISTERED\n");
		}
		else // Component not registered yet, safe to register :)
		{
			componentManagers[typeID] = std::make_unique<ComponentManager<Component>>();
			componentTypeNames.emplace(name, typeID);
			componentTypeList.push_back(typeID);

			if (dependencies.empty())
			{
				return;
			}

			for (const std::string& dep : dependencies)
			{
				std::type_index depTypeIdx = Get_ComponentTypeIndex(dep);
				if (componentManagers.find(depTypeIdx) == componentManagers.end())
				{
					throw std::runtime_error("DEPENDENCY FOR " + name + ", " + dep + " NOT REGISTERED\n");
				}
				componentDependencies[typeID].push_back(depTypeIdx);
				componentDependents[depTypeIdx].push_back(typeID);
			}
		}
	}

	/**
	 * @brief Adds a component of the specified type to an entity.
	 *
	 * Ensures that all declared dependencies for this component type are
	 * satisfied before adding. If dependencies are missing, no component
	 * is added.
	 *
	 * @tparam Component The component type.
	 * @param eID Entity to add the component to.
	 * @param component Optional instance to copy into the manager.
	 */
	template<typename Component>
	void Add_Component(Entity eID, const Component& component = {})
	{
		if (!HasDependencies(eID, typeid(Component)))
		{ return; }

		ComponentManager<Component>* manager = Get_Manager<Component>();
		// Check if this component has dependencies
		manager->addComponent(eID, component);
	}

	/**
	 * @brief Adds a component using its runtime type_index.
	 *
	 * Ensures that all declared dependencies for this component type are
	 * satisfied before adding. If dependencies are missing, no component
	 * is added.
	 * Constructs a default instance using the component manager.
	 *
	 * @param eID Target entity ID.
	 * @param typeID Runtime type of the component (must be registered).
	 */
	void Add_Component(Entity eID, std::type_index typeID)
	{
		IsComponentRegistered(typeID);
		if (!HasDependencies(eID, typeID))
		{ return; }

		componentManagers[typeID]->addComponent(eID);
	}

	/**
	 * @brief Removes a component of the given type from an entity.
	 *
	 * @tparam Component Component type to remove.
	 * @param eID Entity ID.
	 *
	 * @note Removal is prevented if another component depends on this one.
	 */
	template<typename Component>
	void Remove_Component(Entity eID)
	{
		if (HasDependents(eID, typeid(Component)))
		{ return; }

		ComponentManager<Component>* manager = Get_Manager<Component>();

		manager->removeComponent(eID);
	}

	/**
	 * @brief Removes a component using its runtime type_index.
	 * @param eID Entity ID.
	 * @param typeID Runtime type identifier of the component.
	 *
	 * @note Removal is prevented if another component depends on this one.
	 */
	void Remove_Component(Entity eID, std::type_index typeID)
	{
		IsComponentRegistered(typeID);
		if (HasDependents(eID, typeID))
		{ return; }

		componentManagers[typeID]->removeComponent(eID);
	}

	/**
	 * @brief Checks whether an entity has a component of the given type.
	 *
	 * @tparam Component Component type to check for.
	 * @param eID Entity ID.
	 * @return True if the entity has the component.
	 */
	template<typename Component>
	bool Has_Component(Entity eID)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->hasComponent(eID);
	}

	/**
	 * @brief Checks whether an entity has a component of the given type.
	 *
	 * @param eID Entity ID.
	 * @param typeID type_index of Component Type.
	 * @return True if the entity has the component.
	 */
	bool Has_Component(Entity eID, std::type_index typeID)
	{
		IsComponentRegistered(typeID);
		return componentManagers[typeID]->hasComponent(eID);
	}

	/**
	 * @brief Retrieves the Component of an entity.
	 * @param eID Entity identifier.
	 * @return Reference to the entity s Component.
	 */
	template<typename Component>
	Component& Get_Component(Entity eID)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->getComponent(eID);
	}

	/**
	 * @brief Retrieves the entire component map for a specific type.
	 *
	 * @tparam Component Component type.
	 * @return Reference to an unordered_map mapping Entity ? Component.
	 */
	template<typename Component>
	std::unordered_map<Entity, Component>& Get_ComponentMap()
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->getComponents();
	}

	/**
	 * @brief Serializes a single component attached to an entity.
	 *
	 * @tparam Component Component type.
	 * @param eID Entity ID.
	 * @return Optional string of serialized component data.
	 */
	template<typename Component>
	std::optional<std::string> SaveComponent(Entity eID)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->SaveComponent(eID);
	}

	/**
	 * @brief Loads a serialized component and attaches it to an entity.
	 *
	 * @tparam Component Component type.
	 * @param eID Entity ID.
	 * @param data Serialized component data.
	 */
	template<typename Component>
	void LoadComponent(Entity eID, const std::string& data)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		manager->LoadComponent(eID, data);
	}

	/**
	 * @brief Retrieves a component's registered name from its type_index.
	 * @param typeIndex Component runtime type.
	 * @return Human-readable component name.
	 */
	std::string Get_ComponentTypeName(std::type_index typeIndex)
	{
		for (auto it : componentTypeNames)
		{
			if (typeIndex == it.second)
			{
				return it.first;
			}
		}
		// return "No such component name... :(\n";
		throw std::runtime_error("Component name not registered properly\n");
	}

	/**
	 * @brief Retrieves a component's type_index from its registered name.
	 * @param name Component name.
	 * @return std::type_index of the component.
	 */
	std::type_index Get_ComponentTypeIndex(std::string name)
	{
		if (componentTypeNames.find(name) == componentTypeNames.end())
		{
			throw std::runtime_error("CANNOT FIND COMPONENT TYPE " + name);
		}
		return componentTypeNames.at(name);
	}
#pragma endregion
	// ================================================================================

	// ENTITY
	// ================================================================================
#pragma region ENTITY
	/**
	 * @brief Creates a new entity.
	 * @return The unique ID of the created entity.
	 */
	Entity Create_Entity() { return entityManager.createEntity(); }

	/**
	 * @brief Removes an entity and all of its components.
	 * @param eID The entity ID to remove.
	 */
	void Remove_Entity(Entity eID)
	{
		// Remove every component!!
		for (auto& [type, manager] : componentManagers)
		{
			manager->removeComponent(eID);
		}

		entityManager.destroyEntity(eID);
	}

	/**
	 * @brief Removes all entities in the registry.
	 */
	void Remove_AllEntities()
	{
		for (Entity eID = 0; eID < entityManager.maxEntities(); eID++)   
		{
			Remove_Entity(eID);
		}
	}

	/**
	 * @brief Retrieves the global entity?name map.
	 * @return Reference to name map.
	 */
	std::unordered_map<Entity, std::string>& Get_EntityNameMap()
	{
		return entityManager.GetEntityNameMap();
	}

	/**
	 * @brief Assigns a human-readable name to an entity.
	 * @param eID Entity ID.
	 * @param name Name to assign.
	 */
	void Set_EntityName(Entity eID, const std::string& name)
	{
		entityManager.SetEntityName(eID, name);
	}

	/**
	 * @brief Gets the human-readable name of an entity.
	 * @param eID Entity ID.
	 * @return The entity's name.
	 */
	std::string Get_EntityName(Entity eID)
	{
		return entityManager.GetEntityName(eID);
	}

	/**
	 * @brief Retrieves the entity ID associated with a name.
	 * @param name Entity name.
	 * @return Entity ID.
	 */
	Entity Get_EntityID(const std::string& name)
	{
		return entityManager.GetEntityFromName(name);
	}
#pragma endregion
	// ================================================================================

	// EDITOR
	// ================================================================================
#pragma region EDITOR
	/**
	 * @brief Returns the order in which components were registered.
	 * @return Vector of component type_index.
	 */
	const std::vector<std::type_index>& Get_ComponentOrder() {
		return componentTypeList;
	}

	/**
	 * @brief Generates editor metadata for a given entity.
	 * @param eID Entity ID.
	 * @return List of ComponentData structures.
	 */
	std::vector<Editor::ComponentData> Get_EntityData(Entity eID);
#pragma endregion
	// ================================================================================

	// SERIALIZATION
	// ================================================================================
#pragma region SERIALIZATION
	/**
	 * @brief Saves all components of an entity into a SavedEntity structure.
	 * @param eID Entity ID.
	 * @return Serialized entity.
	 */
	SavedEntity Save_Entity(Entity eID)
	{
		SavedEntity saved;
		saved.name = Get_EntityName(eID);

		for (std::type_index typeID : componentTypeList)
		{
			if (!componentManagers[typeID]->hasComponent(eID)) { continue; }
			std::string compType = Get_ComponentTypeName(typeID);
			saved.componentData[compType] = componentManagers[typeID]->SaveComponent(eID);
		}
		return saved;
	}

	/**
	 * @brief Reconstructs an entity and its components from serialized data.
	 * @param saved SavedEntity struct.
	 * @return Newly created entity ID.
	 */
	Entity Load_Entity(const SavedEntity& saved)
	{
		Entity eID = Create_Entity();
		Set_EntityName(eID, saved.name);

		std::cout << "Creating entity '" << saved.name << "' (ID: " << eID << ")\n";

		for (std::type_index typeID : componentTypeList)
		{
			std::string typeName = Get_ComponentTypeName(typeID);
			auto it = saved.componentData.find(typeName);
			if (it != saved.componentData.end())
			{
				componentManagers[typeID]->LoadComponent(eID, it->second);
			}
		}

		return eID;
	}
#pragma endregion
	// ================================================================================

};

