#pragma once
// Tis shid the glue between the E the C and the S so u could call it registry == ECS lololol

// Use entity manager
// Attach components to entities
// Track which entity has what components
// Help systems find stuff in entities
#include "ComponentManager.h"
#include "EntityManager.h"
#include "Component.h"

constexpr size_t MAX_COMPONENT_TYPES = 8; // max number of components

/*
COMPONENTS LIST: (ENUM IN COMPONENT.H)
	1 | CT_Transform	| For position, rotate, and scale data
	2 | CT_Sprite		| Sprite texture data, sprite frame
	3 | CT_Animator		| Animation handler
	4 | CT_Physics		| Collision, hitbox
	5 | CT_Audio
	6 | CT_Lifetime
	7 | CT_Health
	8 | CT_Tag
*/


// Entity functions
Entity Create_Entity();

void Remove_Entity(Entity eID);


// overloads for adding
void Add_Component(Entity eID, TransformComponent	const& comp);
void Add_Component(Entity eID, SpriteComponent		const& comp);
void Add_Component(Entity eID, AnimatorComponent	const& comp);
void Add_Component(Entity eID, PhysicsComponent		const& comp);
void Add_Component(Entity eID, AudioComponent		const& comp);
void Add_Component(Entity eID, LifetimeComponent	const& comp);
void Add_Component(Entity eID, HealthComponent		const& comp);
void Add_Component(Entity eID, TagComponent			const& comp);
void Add_Component(Entity eID, InputComponent		const& comp);
void Add_Component(Entity eID, BossGhoulComponent	const& comp);

// overloads for removal
void Remove_Component(Entity eID, ComponentTypeID const& cID);
void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist);

// Checks if an entity has component of certain type
bool Has_Component(Entity eID, ComponentTypeID const& cID);

// Gets the component of the entity from the specified type
TransformComponent	& Get_TransformComponent(Entity eID);
SpriteComponent		& Get_SpriteComponent(Entity eID);
AnimatorComponent	& Get_AnimatorComponent(Entity eID);
PhysicsComponent	& Get_PhysicsComponent(Entity eID);
AudioComponent		& Get_AudioComponent(Entity eID);
LifetimeComponent	& Get_LifetimeComponent(Entity eID);
HealthComponent		& Get_HealthComponent(Entity eID);
TagComponent		& Get_TagComponent(Entity eID);
InputComponent		& Get_InputComponent(Entity eID);
BossGhoulComponent	& Get_BossGhoulComponent(Entity eID);

// Manager accessors
ComponentManager<TransformComponent>	& CM_Transform();
ComponentManager<SpriteComponent>		& CM_Sprite();
ComponentManager<AnimatorComponent>		& CM_Animator();
ComponentManager<PhysicsComponent>		& CM_Physics();
ComponentManager<AudioComponent	>		& CM_Audio();
ComponentManager<LifetimeComponent>		& CM_Lifetime();
ComponentManager<HealthComponent>		& CM_Health();
ComponentManager<TagComponent>			& CM_Tag();
ComponentManager<InputComponent>		& CM_Input();
ComponentManager<BossGhoulComponent>	& CM_BossGhoul();

// Map Accessors


//extern ComponentManager<TransformComponent> transform;
//extern ComponentManager<SpriteComponent>    sprite;
//extern ComponentManager<AnimatorComponent>  animator;
//extern ComponentManager<PhysicsComponent>   physics;
//extern ComponentManager<AudioComponent>     audio;
//extern ComponentManager<LifetimeComponent>  lifetime;
//extern ComponentManager<HealthComponent>    health;
//extern ComponentManager<TagComponent>       tag;
//extern ComponentManager<InputComponent>		input;
//extern ComponentManager<BossGhoulComponent> bossghoul;

//template<typename T>
//bool HasComponent(Entity e);
//
//template<typename T>
//T& GetComponent(Entity e);

//// Transform
//template<>
//inline bool HasComponent<TransformComponent>(Entity e) { return transform.hasComponent(e); }
//
//template<>
//inline TransformComponent& GetComponent<TransformComponent>(Entity e) { return transform.getComponent(e); }
//
//// Physics
//template<>
//inline bool HasComponent<PhysicsComponent>(Entity e) { return physics.hasComponent(e); }
//
//template<>
//inline PhysicsComponent& GetComponent<PhysicsComponent>(Entity e) { return physics.getComponent(e); }
//
//// Sprite
//template<>
//inline bool HasComponent<SpriteComponent>(Entity e) { return sprite.hasComponent(e); }
//
//template<>
//inline SpriteComponent& GetComponent<SpriteComponent>(Entity e) { return sprite.getComponent(e); }
//
//// Tag
//template<>
//inline bool HasComponent<TagComponent>(Entity e) { return tag.hasComponent(e); }
//
//template<>
//inline TagComponent& GetComponent<TagComponent>(Entity e) { return tag.getComponent(e); }
//
//// Input
//template<>
//inline bool HasComponent<InputComponent>(Entity e) { return input.hasComponent(e); }
//
//template<>
//inline InputComponent& GetComponent<InputComponent>(Entity e) { return input.getComponent(e); }
//
//template<>
//inline bool HasComponent<BossGhoulComponent>(Entity e) { return bossghoul.hasComponent(e); }
//
//template<>
//inline BossGhoulComponent& GetComponent<BossGhoulComponent>(Entity e) { return bossghoul.getComponent(e); }
//
//// Health
//template<>
//inline bool HasComponent<HealthComponent>(Entity e) { return health.hasComponent(e); }
//
//template<>
//inline HealthComponent& GetComponent<HealthComponent>(Entity e) { return health.getComponent(e); }
//

