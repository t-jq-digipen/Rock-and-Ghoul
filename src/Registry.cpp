#include "Registry.h"


// ENTITY REGION
#pragma region ENTITY
Entity Registry::Add_Entity()
{
	return entityManager.createEntity();
}


void Registry::Remove_Entity(Entity eID)
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

void Registry::Add_Component(Entity eID, TransformComponent const& comp)
{
	transform.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, SpriteComponent const& comp)
{
	sprite.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, AnimatorComponent const& comp)
{
	animator.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, PhysicsComponent const& comp)
{
	physics.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, AudioComponent const& comp)
{
	audio.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, LifetimeComponent const& comp)
{
	lifetime.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, HealthComponent const& comp)
{
	health.addComponent(eID, comp);
}

void Registry::Add_Component(Entity eID, TagComponent const& comp)
{
	tag.addComponent(eID, comp);
}

#pragma endregion




// REMOVE

void Registry::Remove_Component(Entity eID, ComponentTypeID cID)
{
	switch (cID)
	{
	case (CT_None):
	{
		break;
	}
	case (CT_Transform):
	{
		transform.removeComponent(eID);
	}
	break;	// CT_Transform

	case (CT_Sprite):
	{
		sprite.removeComponent(eID);
	}
	break;	// CT_Sprite

	case (CT_Animator):
	{
		animator.removeComponent(eID);
	}
	break;	// CT_Animator

	case (CT_Physics):
	{
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
		tag.removeComponent(eID);
	}
	break;	// CT_Tag
	}
}


void Registry::Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist)
{
	for (ComponentTypeID cID : cIDlist)
	{
		Remove_Component(eID, cID);
	}
}
#pragma endregion COMPONENT
