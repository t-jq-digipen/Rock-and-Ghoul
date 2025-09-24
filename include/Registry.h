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

class Registry {
private:
	// masks? food for thought
	std::vector<ComponentMask> entityMasks;

	// Entity manager (stores a REFERENCE to the singleton entity manager ^^)
	EntityManager& entityManager = EntityManager::get();

	

	// CT_Transform
	ComponentManager<TransformComponent>	transform;

	// Graphics
	// CT_Sprite
	ComponentManager<SpriteComponent>		sprite;
	// CT_Animator
	ComponentManager<AnimatorComponent>		animator;

	// Physics
	// CT_Physics
	ComponentManager<PhysicsComponent>		physics;

	// Audio
	// CT_Audio
	ComponentManager<AudioComponent>		audio;

	// Timer
	// CT_Lifetime
	ComponentManager<LifetimeComponent>		lifetime;

	// Stats
	// CT_Health
	ComponentManager<HealthComponent>		health;

	// Tag
	// CT_Tag
	ComponentManager<TagComponent>			tag;

public:
	Registry() = default;

	Entity Add_Entity();
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

};
