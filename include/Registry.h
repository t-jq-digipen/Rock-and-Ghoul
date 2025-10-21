/* ========================================================
 * @file		Registry.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi  | yasmin.t@digipen.edu

 * @date		01 OCT 2025
 * 
 * @brief	Registry for ECS management
 *
 * Declares interface functions for Entity and Component
 * management of the ECS architecture. Provides functions
 * for creating, removing, and accessing entities and
 * their components, as well as accessors for the
 * component managers.
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
#include "ComponentManager.h"
#include "EntityManager.h"
#include "Component.h"
#include "Physics.h"		// Physics Component
#include "Sprite.h"			// Sprite Component
#include "Animator.h"		// Animator Component
#include "TextSystem.h"		// Text Component

// constexpr size_t MAX_COMPONENT_TYPES = 11; // max number of components TIS SHI NOT EVEN BEING USED


// Entity functions
// ========================================================
/**
 * @brief Creates a new entity.
 * @return The unique ID of the created entity.
 */
Entity Create_Entity();
/**
 * @brief Removes an entity and all of its components.
 * @param eID The entity ID to remove.
 */
void Remove_Entity(Entity eID);
// ======================================================== //


// overloads for adding
/**
 * @brief Adds a Component to an entity.
 * @param eID Entity identifier.
 * @param comp Component instance to add.
 */
// ========================================================
void Add_Component(Entity eID, TransformComponent			const& comp);
void Add_Component(Entity eID, Sprite::SpriteComponent		const& comp);
void Add_Component(Entity eID, Animator::AnimatorComponent	const& comp);
void Add_Component(Entity eID, Physics::PhysicsComponent	const& comp);
void Add_Component(Entity eID, AudioComponent				const& comp);
void Add_Component(Entity eID, LifetimeComponent			const& comp);
void Add_Component(Entity eID, HealthComponent				const& comp);
void Add_Component(Entity eID, TagComponent					const& comp);
void Add_Component(Entity eID, InputComponent				const& comp);
void Add_Component(Entity eID, ShapeComponent				const& comp);
void Add_Component(Entity eID, TextSystem::TextComponent	const& comp);
// ======================================================== //

// removal
// ========================================================
/**
 * @brief Removes a specific component type from an entity.
 * @param eID Entity identifier.
 * @param cID Component type identifier.
 */
void Remove_Component(Entity eID, ComponentTypeID const& cID);
/**
 * @brief Removes a specific component type from an entity.
 * @param eID Entity identifier.
 * @param cIDlist Initializer list of Component type identifiers
 */
void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist);
// ======================================================== //

/**
 * @brief Checks whether an entity has the given component type.
 * @param eID Entity identifier.
 * @param cID Component type identifier.
 * @return True if the entity has the component, false otherwise.
 */
bool Has_Component(Entity eID, ComponentTypeID const& cID);

// Gets the component of the entity from the specified type
/**
 * @brief Retrieves the Component of an entity.
 * @param eID Entity identifier.
 * @return Reference to the entity’s Component.
 */
// ========================================================
TransformComponent			& Get_TransformComponent(Entity eID);
Sprite::SpriteComponent		& Get_SpriteComponent(Entity eID);
Animator::AnimatorComponent	& Get_AnimatorComponent(Entity eID);
Physics::PhysicsComponent	& Get_PhysicsComponent(Entity eID);
AudioComponent				& Get_AudioComponent(Entity eID);
LifetimeComponent			& Get_LifetimeComponent(Entity eID);
HealthComponent				& Get_HealthComponent(Entity eID);
TagComponent				& Get_TagComponent(Entity eID);
InputComponent				& Get_InputComponent(Entity eID);
ShapeComponent				& Get_ShapeComponent(Entity eID);
TextSystem::TextComponent	& Get_TextComponent(Entity eID);
// ======================================================== //

// Manager accessors
/**
 * @brief Retrieves the Manager of a component.
 * @return Reference to the Component's manager.
 */
// ========================================================
ComponentManager<TransformComponent>			& CM_Transform();
ComponentManager<Sprite::SpriteComponent>		& CM_Sprite();
ComponentManager<Animator::AnimatorComponent>	& CM_Animator();
ComponentManager<Physics::PhysicsComponent>		& CM_Physics();
ComponentManager<AudioComponent>				& CM_Audio();
ComponentManager<LifetimeComponent>				& CM_Lifetime();
ComponentManager<HealthComponent>				& CM_Health();
ComponentManager<TagComponent>					& CM_Tag();
ComponentManager<InputComponent>				& CM_Input();
ComponentManager<ShapeComponent>				& CM_Shape();
ComponentManager<TextSystem::TextComponent>		& CM_Text();
// ======================================================== //

// Component list accessor?

/**
 * @brief Returns reference to the static entity name map singleton.
 *
 *
 * @return std::unordered_map<Entity, std::string>& Reference to the name map.
 */std::unordered_map<Entity, std::string>& GetEntityNameMap();
 
 /**
 * @brief Assigns a nice readable name to an entity
 *
 * Used for debugging, serialization, and editor display.
 *
 * @param eID The entity ID to name.
 * @param name The string name to assign (e.g., "Player").
 */
void SetEntityName(Entity eID, const std::string& name);

/**
 * @brief Retrieves the name associated with an entity.
 *
 * Returns the custom name if one was set via SetEntityName(), otherwise
 * generates a default name in the format "Entity_{ID}".
 *
 * @param eID The entity ID to query.
 * @return std::string The entity's name or auto-generated fallback.
 */
std::string GetEntityName(Entity eID);