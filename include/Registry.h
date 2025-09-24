#pragma once
// Tis shid the glue between the E the C and the S so u could call it registry == ECS lololol

// Use entity manager
// Attach components to entities
// Track which entity has what components
// Help systems find stuff in entities
#include "ComponentManager.h"
#include "EntityManager.h"


constexpr size_t MAX_COMPONENT_TYPES = 8; // max number of components
/*
COMPONENTS LIST:
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
void Add_Component(Entity eID, TransformComponent const& comp);
void Add_Component(Entity eID, SpriteComponent const& comp);
void Add_Component(Entity eID, AnimatorComponent const& comp);
void Add_Component(Entity eID, PhysicsComponent const& comp);
void Add_Component(Entity eID, AudioComponent const& comp);
void Add_Component(Entity eID, LifetimeComponent const& comp);
void Add_Component(Entity eID, HealthComponent const& comp);
void Add_Component(Entity eID, TagComponent const& comp);

// overloads for removal
void Remove_Component(Entity eID, ComponentTypeID cID);
void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist);