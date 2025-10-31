/* ========================================================
 * @file		Registry.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi  | yasmin.t@digipen.edu

 * @date		23 OCT 2025
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
 * Copyright   2025 DigiPen, All rights reserved.
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
#include "DataDefs.h"
#include <nlohmann/json.hpp>

// constexpr size_t MAX_COMPONENT_TYPES = 11; // max number of components TIS SHI NOT EVEN BEING USED

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

	template<typename Component>
	ComponentManager<Component>* Get_Manager() {
		auto it = componentManagers.find(std::type_index(typeid(Component)));
		if (it == componentManagers.end())
		{
			throw std::runtime_error("COMPONENT NOT REGISTERED ???\n");
		}

		return static_cast<ComponentManager<Component>*>(it->second.get());
	}

	//template<typename Component>
	//void Register_ComponentTypeName(const std::string& name)
	//{
	//	std::type_index typeIndex(typeid(Component));

	//	if (componentTypeNames.find(name) != componentTypeNames.end())
	//	{
	//		throw std::runtime_error("COMPONENT NAME " + name + " ALREADY REGISTERED\n");
	//	}

	//	componentTypeNames[name] = typeIndex;
	//}

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
				std::cout << "Entity " << eID << " Dependency type " << Get_ComponentTypeName(dep) << " for " << Get_ComponentTypeName(typeID) << " not found\n";
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
				std::cout << "Entity " << eID << "Component type " << Get_ComponentTypeName(reliant) << " is relying on " << Get_ComponentTypeName(typeID) << '\n';
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
	 * @brief Adds a Component to an entity.
	 * @param eID Entity identifier.
	 * @param component Component instance to add.
	 */
	template<typename Component>
	void Add_Component(Entity eID, const Component& component = {})
	{
		if (!HasDependencies(eID, typeid(Component)))
		{
			return;
		}
		ComponentManager<Component>* manager = Get_Manager<Component>();
		// Check if this component has dependencies
		//auto deplist = componentDependencies.find(typeid(Component));
		/*if (deplist != componentDependencies.end())
		{
			for (std::type_index dep : deplist->second)
			{
			}
		}*/
		manager->addComponent(eID, component);
	}

	// add default comp using typeID
	void Add_Component(Entity eID, std::type_index typeID)
	{
		IsComponentRegistered(typeID);
		if (!HasDependencies(eID, typeID))
		{
			return;
		}
		componentManagers[typeID]->addComponent(eID);
	}

	/**
	 * @brief Removes a specific component type from an entity.
	 * @param eID Entity identifier.
	 */
	template<typename Component>
	void Remove_Component(Entity eID)
	{
		if (HasDependents(eID, typeid(Component)))
		{
			return;
		}

		ComponentManager<Component>* manager = Get_Manager<Component>();

		manager->removeComponent(eID);
	}

	// remove comp using typeID
	void Remove_Component(Entity eID, std::type_index typeID)
	{
		IsComponentRegistered(typeID);
		if (HasDependents(eID, typeID))
		{
			return;
		}

		componentManagers[typeID]->removeComponent(eID);
	}

	template<typename Component>
	bool Has_Component(Entity eID)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->hasComponent(eID);
	}

	// check has component using typeID
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

	template<typename Component>
	std::unordered_map<Entity, Component>& Get_ComponentMap()
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->getComponents();
	}

	template<typename Component>
	std::optional<std::string> SaveComponent(Entity eID)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		return manager->SaveComponent(eID);
	}

	template<typename Component>
	void LoadComponent(Entity eID, const std::string& data)
	{
		ComponentManager<Component>* manager = Get_Manager<Component>();
		manager->LoadComponent(eID, data);
	}


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

	std::unordered_map<Entity, std::string>& Get_EntityNameMap()
	{
		return entityManager.GetEntityNameMap();
	}

	void Set_EntityName(Entity eID, const std::string& name)
	{
		entityManager.SetEntityName(eID, name);
	}

	std::string Get_EntityName(Entity eID)
	{
		return entityManager.GetEntityName(eID);
	}

	Entity Get_EntityID(const std::string& name)
	{
		return entityManager.GetEntityFromName(name);
	}
#pragma endregion
	// ================================================================================

	// EDITOR
	// ================================================================================
#pragma region EDITOR
	const std::vector<std::type_index>& Get_ComponentOrder() {
		return componentTypeList;
	}

	std::vector<Editor::ComponentData> Get_EntityData(Entity eID);
#pragma endregion
	// ================================================================================

	// SERIALIZATION
	// ================================================================================
#pragma region SERIALIZATION
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
		/*
		for (auto& [typeIndex, manager] : componentManagers)
		{
			std::string compType = Get_ComponentTypeName(typeIndex);
			saved.componentData[compType] = manager->SaveComponent(eID);
		}*/
		return saved;
	}

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

		//for (auto& [typeName, data] : saved.componentData)
		//{
		//	std::type_index typeIndex(Get_ComponentTypeIndex(typeName));
		//	componentManagers[typeIndex]->LoadComponent(eID, data);
		//}

		return eID;
	}
#pragma endregion
	// ================================================================================





};


//// Entity functions
//// ========================================================
///**
// * @brief Creates a new entity.
// * @return The unique ID of the created entity.
// */
//Entity Create_Entity();
///**
// * @brief Removes an entity and all of its components.
// * @param eID The entity ID to remove.
// */
//void Remove_Entity(Entity eID);
//// ======================================================== //
//
//
//// overloads for adding
///**
// * @brief Adds a Component to an entity.
// * @param eID Entity identifier.
// * @param comp Component instance to add.
// */
//// ========================================================
//void Add_Component(Entity eID, TransformComponent			const& comp);
//void Add_Component(Entity eID, Sprite::SpriteComponent		const& comp);
//void Add_Component(Entity eID, Animator::AnimatorComponent	const& comp);
//void Add_Component(Entity eID, Physics::PhysicsComponent	const& comp);
//void Add_Component(Entity eID, AudioComponent				const& comp);
//void Add_Component(Entity eID, LifetimeComponent			const& comp);
//void Add_Component(Entity eID, HealthComponent				const& comp);
//void Add_Component(Entity eID, TagComponent					const& comp);
//void Add_Component(Entity eID, InputComponent				const& comp);
//void Add_Component(Entity eID, ShapeComponent				const& comp);
//void Add_Component(Entity eID, ElementComponent				const& comp);
//// ======================================================== //
//
//// removal
//// ========================================================
///**
// * @brief Removes a specific component type from an entity.
// * @param eID Entity identifier.
// * @param cID Component type identifier.
// */
//void Remove_Component(Entity eID, ComponentTypeID const& cID);
///**
// * @brief Removes a specific component type from an entity.
// * @param eID Entity identifier.
// * @param cIDlist Initializer list of Component type identifiers
// */
//void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist);
//// ======================================================== //
//
///**
// * @brief Checks whether an entity has the given component type.
// * @param eID Entity identifier.
// * @param cID Component type identifier.
// * @return True if the entity has the component, false otherwise.
// */
//bool Has_Component(Entity eID, ComponentTypeID const& cID);
//
//// Gets the component of the entity from the specified type
///**
// * @brief Retrieves the Component of an entity.
// * @param eID Entity identifier.
// * @return Reference to the entity s Component.
// */
//// ========================================================
//TransformComponent			& Get_TransformComponent(Entity eID);
//Sprite::SpriteComponent		& Get_SpriteComponent(Entity eID);
//Animator::AnimatorComponent	& Get_AnimatorComponent(Entity eID);
//Physics::PhysicsComponent	& Get_PhysicsComponent(Entity eID);
//AudioComponent				& Get_AudioComponent(Entity eID);
//LifetimeComponent			& Get_LifetimeComponent(Entity eID);
//HealthComponent				& Get_HealthComponent(Entity eID);
//TagComponent				& Get_TagComponent(Entity eID);
//InputComponent				& Get_InputComponent(Entity eID);
//ShapeComponent				& Get_ShapeComponent(Entity eID);
//ElementComponent			& Get_ElementComponent(Entity eID);
//// ======================================================== //
//
//// Manager accessors
///**
// * @brief Retrieves the Manager of a component.
// * @return Reference to the Component's manager.
// */
//// ========================================================
//ComponentManager<TransformComponent>			& CM_Transform();
//ComponentManager<Sprite::SpriteComponent>		& CM_Sprite();
//ComponentManager<Animator::AnimatorComponent>	& CM_Animator();
//ComponentManager<Physics::PhysicsComponent>		& CM_Physics();
//ComponentManager<AudioComponent>				& CM_Audio();
//ComponentManager<LifetimeComponent>				& CM_Lifetime();
//ComponentManager<HealthComponent>				& CM_Health();
//ComponentManager<TagComponent>					& CM_Tag();
//ComponentManager<InputComponent>				& CM_Input();
//ComponentManager<ShapeComponent>				& CM_Shape();
//ComponentManager<ElementComponent>				& CM_Element();
//// ======================================================== //

// Component list accessor?

/**
 * @brief Returns reference to the static entity name map singleton.
 *
 *
 * @return std::unordered_map<Entity, std::string>& Reference to the name map.
 */
 //std::unordered_map<Entity, std::string>& GetEntityNameMap();
 // 
 // /**
 // * @brief Assigns a nice readable name to an entity
 // *
 // * Used for debugging, serialization, and editor display.
 // *
 // * @param eID The entity ID to name.
 // * @param name The string name to assign (e.g., "Player").
 // */
 //void SetEntityName(Entity eID, const std::string& name);
 //
 ///**
 // * @brief Retrieves the name associated with an entity.
 // *
 // * Returns the custom name if one was set via SetEntityName(), otherwise
 // * generates a default name in the format "Entity_{ID}".
 // *
 // * @param eID The entity ID to query.
 // * @return std::string The entity's name or auto-generated fallback.
 // */
 //std::string GetEntityName(Entity eID);