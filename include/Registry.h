#pragma once
// Tis shid the glue between the E the C and the S so u could call it registry == ECS lololol

// Use entity manager
// Attach components to entities
// Track which entity has what components
// Help systems find stuff in entities
#include "ComponentManager.h"
#include "EntityManager.h"
#include "Component.h"

// constexpr size_t MAX_COMPONENT_TYPES = 11; // max number of components TIS SHI NOT EVEN BEING USED

/*
COMPONENTS LIST: (ENUM IN COMPONENT.H)
	1 | CT_Transform		| For position, rotate, and scale data
	2 | CT_Sprite			| Sprite texture data, sprite frame
	3 | CT_Animator			| Animation handler
	4 | CT_Physics			| Velocity, acceleration
	5 | CT_AABB				| AABB collision hitbox
	5 | CT_CircleCollider	| Circle collision hitbox
	6 | CT_Audio
	7 | CT_Lifetime
	8 | CT_Health
	9 | CT_Tag
*/


// Entity functions
Entity Create_Entity();

void Remove_Entity(Entity eID);


// overloads for adding
void Add_Component(Entity eID, TransformComponent		const& comp);
void Add_Component(Entity eID, SpriteComponent			const& comp);
void Add_Component(Entity eID, AnimatorComponent		const& comp);
void Add_Component(Entity eID, PhysicsComponent			const& comp);
void Add_Component(Entity eID, AABBComponent			const& comp);
void Add_Component(Entity eID, CircleColliderComponent	const& comp);
void Add_Component(Entity eID, AudioComponent			const& comp);
void Add_Component(Entity eID, LifetimeComponent		const& comp);
void Add_Component(Entity eID, HealthComponent			const& comp);
void Add_Component(Entity eID, TagComponent				const& comp);
void Add_Component(Entity eID, InputComponent			const& comp);

// overloads for removal
void Remove_Component(Entity eID, ComponentTypeID const& cID);
void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist);

// Checks if an entity has component of certain type
bool Has_Component(Entity eID, ComponentTypeID const& cID);

// Gets the component of the entity from the specified type
TransformComponent		& Get_TransformComponent(Entity eID);
SpriteComponent			& Get_SpriteComponent(Entity eID);
AnimatorComponent		& Get_AnimatorComponent(Entity eID);
PhysicsComponent		& Get_PhysicsComponent(Entity eID);
AABBComponent			& Get_AABBComponent(Entity eID);
CircleColliderComponent	& Get_CircleColliderComponent(Entity eID);
AudioComponent			& Get_AudioComponent(Entity eID);
LifetimeComponent		& Get_LifetimeComponent(Entity eID);
HealthComponent			& Get_HealthComponent(Entity eID);
TagComponent			& Get_TagComponent(Entity eID);
InputComponent			& Get_InputComponent(Entity eID);

// Manager accessors
ComponentManager<TransformComponent>		& CM_Transform();
ComponentManager<SpriteComponent>			& CM_Sprite();
ComponentManager<AnimatorComponent>			& CM_Animator();
ComponentManager<PhysicsComponent>			& CM_Physics();
ComponentManager<AABBComponent>				& CM_AABB();
ComponentManager<CircleColliderComponent>	& CM_CircleCollider();
ComponentManager<AudioComponent	>			& CM_Audio();
ComponentManager<LifetimeComponent>			& CM_Lifetime();
ComponentManager<HealthComponent>			& CM_Health();
ComponentManager<TagComponent>				& CM_Tag();
ComponentManager<InputComponent>			& CM_Input();

