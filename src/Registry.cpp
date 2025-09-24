#include "Registry.h"




// COMPONENT MANAGERS
// ================================================================================
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


// Entity manager (stores a REFERENCE to the singleton entity manager ^^)
// ================================================================================
EntityManager& entityManager = EntityManager::get();

// ENTITY REGION
#pragma region ENTITY

Entity Create_Entity()
{
	std::cout << "Entity creating!\n";
	return entityManager.createEntity();
}


void Remove_Entity(Entity eID)
{
	// remove all components!
	Remove_Components(eID, { CT_Transform, CT_Sprite, CT_Animator, CT_Physics, CT_Audio, CT_Lifetime, CT_Health, CT_Tag });

	// remove entity ID!
	entityManager.destroyEntity(eID);
}
#pragma endregion ENTITY


// COMPONENT REGION
#pragma region COMPONENT
/*
COMPONENTS LIST:
	1 | CT_Transform	| For position, rotate, and scale data
	2 | CT_Sprite		| Sprite texture data, sprite frame
	3 | CT_Animator		| Animation handler
	4 | CT_Physics		| Collision, hitbox
	5 | CT_Audio		| Audio
	6 | CT_Lifetime		| Lifetime
	7 | CT_Health		| Health/Maxhealth
	8 | CT_Tag			| Tag
*/

// ADD
#pragma region ADD COMPONENT

void Add_Component(Entity eID, TransformComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | TRANSFORM | eID: " << eID << "\n";
	transform.addComponent(eID, comp);
}

void Add_Component(Entity eID, SpriteComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | SPRITE    | eID: " << eID << "\n";
	sprite.addComponent(eID, comp);
}

void Add_Component(Entity eID, AnimatorComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | ANIM      | eID: " << eID << "\n";
	animator.addComponent(eID, comp);
}

void Add_Component(Entity eID, PhysicsComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | PHYSICS   | eID: " << eID << "\n";
	physics.addComponent(eID, comp);
}

void Add_Component(Entity eID, AudioComponent const& comp)
{
	audio.addComponent(eID, comp);
}

void Add_Component(Entity eID, LifetimeComponent const& comp)
{
	lifetime.addComponent(eID, comp);
}

void Add_Component(Entity eID, HealthComponent const& comp)
{
	health.addComponent(eID, comp);
}

void Add_Component(Entity eID, TagComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | TAG       | eID: " << eID << "\n";
	tag.addComponent(eID, comp);
}

#pragma endregion




// REMOVE

void Remove_Component(Entity eID, ComponentTypeID cID)
{
	switch (cID)
	{
	case (CT_None):
	{
		break;
	}
	case (CT_Transform):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | TRANSFORM | eID: " << eID << "\n";
		transform.removeComponent(eID);
	}
	break;	// CT_Transform

	case (CT_Sprite):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | SPRITE    | eID: " << eID << "\n";
		sprite.removeComponent(eID);
	}
	break;	// CT_Sprite

	case (CT_Animator):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | ANIM      | eID: " << eID << "\n";
		animator.removeComponent(eID);
	}
	break;	// CT_Animator

	case (CT_Physics):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | PHYSICS   | eID: " << eID << "\n";
		physics.removeComponent(eID);
	}
	break;	// CT_Physics

	case (CT_Audio):
	{
		audio.removeComponent(eID);
	}
	break;	// CT_Audio

	case (CT_Lifetime):
	{
		lifetime.removeComponent(eID);
	}
	break;	// CT_Lifetime

	case (CT_Health):
	{
		health.removeComponent(eID);
	}
	break;	// CT_Health

	case (CT_Tag):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | TAG       | eID: " << eID << "\n";
		tag.removeComponent(eID);
	}
	break;	// CT_Tag
	}
}


void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist)
{
	for (ComponentTypeID cID : cIDlist)
	{
		Remove_Component(eID, cID);
	}
}
#pragma endregion COMPONENT
