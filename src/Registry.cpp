#include "Registry.h"



// COMPONENT MANAGERS
// ================================================================================
// CT_Transform
ComponentManager<TransformComponent>		transform;

// Graphics
// CT_Sprite
ComponentManager<SpriteComponent>			sprite;
// CT_Animator
ComponentManager<AnimatorComponent>			animator;

// Physics
// CT_Physics
ComponentManager<PhysicsComponent>			physics;
// CT_AABB
ComponentManager<AABBComponent>				aabb;
// CT_CircleCollider
ComponentManager<CircleColliderComponent>	circlecollider;

// Audio
// CT_Audio
ComponentManager<AudioComponent>			audio;

// Timer
// CT_Lifetime
ComponentManager<LifetimeComponent>			lifetime;

// Stats
// CT_Health
ComponentManager<HealthComponent>			health;

// Tag
// CT_Tag
ComponentManager<TagComponent>				tag;

ComponentManager<InputComponent>			input;

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
	Remove_Components(eID, { CT_Transform, CT_Sprite, CT_Animator, CT_Physics, CT_AABB, CT_CircleCollider, CT_Audio, CT_Lifetime, CT_Health, CT_Tag });

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
	//std::cout << "REGISTRY | ADDING COMPONENT | TRANSFORM	| eID: " << eID << "\n";
	transform.addComponent(eID, comp);
}

void Add_Component(Entity eID, SpriteComponent const& comp)
{
	//std::cout << "REGISTRY | ADDING COMPONENT | SPRITE		| eID: " << eID << "\n";
	sprite.addComponent(eID, comp);
}

void Add_Component(Entity eID, AnimatorComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | ANIM		| eID: " << eID << "\n";
	animator.addComponent(eID, comp);
}

void Add_Component(Entity eID, PhysicsComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | PHYSICS		| eID: " << eID << "\n";
	physics.addComponent(eID, comp);
}

void Add_Component(Entity eID, AABBComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | AABB		| eID: " << eID << "\n";
	aabb.addComponent(eID, comp);
}

void Add_Component(Entity eID, CircleColliderComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | CRCCOLLIDER	| eID: " << eID << "\n";
	circlecollider.addComponent(eID, comp);
}

void Add_Component(Entity eID, AudioComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | AUDIO		| eID: " << eID << "\n";
	audio.addComponent(eID, comp);
}

void Add_Component(Entity eID, LifetimeComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | LIFETIME	| eID: " << eID << "\n";
	lifetime.addComponent(eID, comp);
}

void Add_Component(Entity eID, HealthComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | HEALTH		| eID: " << eID << "\n";
	health.addComponent(eID, comp);
}

void Add_Component(Entity eID, TagComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | TAG			| eID: " << eID << "\n";
	tag.addComponent(eID, comp);
}
void Add_Component(Entity eID, InputComponent const& comp)
{
	std::cout << "REGISTRY | ADDING COMPONENT | INPUT		| eID: " << eID << "\n";
	input.addComponent(eID, comp);
}


#pragma endregion




// REMOVE

void Remove_Component(Entity eID, ComponentTypeID const& cID)
{
	switch (cID)
	{
	case (CT_None):
	{
		break;
	}
	case (CT_Transform):
	{
		//std::cout << "REGISTRY | REMOVING COMPONENT | TRANSFORM		| eID: " << eID << "\n";
		transform.removeComponent(eID);
	}
	break;	// CT_Transform

	case (CT_Sprite):
	{
		//std::cout << "REGISTRY | REMOVING COMPONENT | SPRITE		| eID: " << eID << "\n";
		sprite.removeComponent(eID);
	}
	break;	// CT_Sprite

	case (CT_Animator):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | ANIM			| eID: " << eID << "\n";
		animator.removeComponent(eID);
	}
	break;	// CT_Animator

	case (CT_Physics):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | PHYSICS		| eID: " << eID << "\n";
		physics.removeComponent(eID);
	}
	break;	// CT_Physics

	case (CT_AABB):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | AABB			| eID: " << eID << "\n";
		aabb.removeComponent(eID);
	}
	break;	// CT_AABB

	case (CT_CircleCollider):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | CRCCOLLIDER	| eID: " << eID << "\n";
		circlecollider.removeComponent(eID);
	}
	break;	// CT_CircleCollider

	case (CT_Audio):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | AUDIO			| eID: " << eID << "\n";
		audio.removeComponent(eID);
	}
	break;	// CT_Audio

	case (CT_Lifetime):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | LIFETIME		| eID: " << eID << "\n";
		lifetime.removeComponent(eID);
	}
	break;	// CT_Lifetime

	case (CT_Health):
	{
		//std::cout << "REGISTRY | REMOVING COMPONENT | HEALTH		| eID: " << eID << "\n";
		health.removeComponent(eID);
	}
	break;	// CT_Health

	case (CT_Tag):
	{
		std::cout << "REGISTRY | REMOVING COMPONENT | TAG			| eID: " << eID << "\n";
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

bool Has_Component(Entity eID, ComponentTypeID const& cID)
{

	switch (cID)
	{
	case (CT_None):
	{
		return false;
		break;
	}
	case (CT_Transform):
	{
		//std::cout << "REGISTRY | HAS COMPONENT | TRANSFORM | eID: " << eID << "\n";
		return transform.hasComponent(eID);
	}
	break;	// CT_Transform

	case (CT_Sprite):
	{
		//std::cout << "REGISTRY | HAS COMPONENT | SPRITE    | eID: " << eID << "\n";
		return sprite.hasComponent(eID);
	}
	break;	// CT_Sprite

	case (CT_Animator):
	{
		std::cout << "REGISTRY | HAS COMPONENT | ANIM      | eID: " << eID << "\n";
		return animator.hasComponent(eID);
	}
	break;	// CT_Animator

	case (CT_Physics):
	{
		//std::cout << "REGISTRY | HAS COMPONENT | PHYSICS   | eID: " << eID << "\n";
		return physics.hasComponent(eID);
	}
	break;	// CT_Physics

	case (CT_AABB):
	{
		std::cout << "REGISTRY | HAS COMPONENT | COLLIDER  | eID: " << eID << "\n";
		return aabb.hasComponent(eID);
	}
	break;	// CT_AABB

	case (CT_CircleCollider):
	{
		std::cout << "REGISTRY | HAS COMPONENT | COLLIDER  | eID: " << eID << "\n";
		return circlecollider.hasComponent(eID);
	}
	break;	// CT_CircleCollider

	case (CT_Audio):
	{
		std::cout << "REGISTRY | HAS COMPONENT | AUDIO     | eID: " << eID << "\n";
		return audio.hasComponent(eID);
	}
	break;	// CT_Audio

	case (CT_Lifetime):
	{
		std::cout << "REGISTRY | HAS COMPONENT | LIFETIME  | eID: " << eID << "\n";
		return lifetime.hasComponent(eID);
	}
	break;	// CT_Lifetime

	case (CT_Health):
	{
		std::cout << "REGISTRY | HAS COMPONENT | HEALTH    | eID: " << eID << "\n";
		return health.hasComponent(eID);
	}
	break;	// CT_Health

	case (CT_Tag):
	{
		std::cout << "REGISTRY | HAS COMPONENT | TAG       | eID: " << eID << "\n";
		return tag.hasComponent(eID);
	}
	break;	// CT_Tag

	case (CT_Input):
	{
		std::cout << "REGISTRY | HAS COMPONENT | TAG       | eID: " << eID << "\n";
		return input.hasComponent(eID);
	}
	break;	// CT_Tag

	default:
		return false;
	}
}


// GET

// Gets the component of the entity from the specified type
TransformComponent&			Get_TransformComponent(Entity eID)		{ return transform.getComponent(eID); }

SpriteComponent&			Get_SpriteComponent(Entity eID)			{ return sprite.getComponent(eID); }

AnimatorComponent&			Get_AnimatorComponent(Entity eID)		{ return animator.getComponent(eID); }

PhysicsComponent&			Get_PhysicsComponent(Entity eID)		{ return physics.getComponent(eID); }

AABBComponent&				Get_AABBComponent(Entity eID)			{ return aabb.getComponent(eID); }

CircleColliderComponent&	Get_CircleColliderComponent(Entity eID)	{ return circlecollider.getComponent(eID); }

AudioComponent&				Get_AudioComponent(Entity eID)			{ return audio.getComponent(eID); }

LifetimeComponent&			Get_LifetimeComponent(Entity eID)		{ return lifetime.getComponent(eID); }

HealthComponent&			Get_HealthComponent(Entity eID)			{ return health.getComponent(eID); }

TagComponent&				Get_TagComponent(Entity eID)			{ return tag.getComponent(eID); }

InputComponent&				Get_InputComponent(Entity eID)			{ return input.getComponent(eID); }


// Manager accessors
ComponentManager<TransformComponent>&		CM_Transform()		{ return transform; }
ComponentManager<SpriteComponent>&			CM_Sprite()			{ return sprite; }
ComponentManager<AnimatorComponent>&		CM_Animator()		{ return animator; }
ComponentManager<PhysicsComponent>&			CM_Physics()		{ return physics; }
ComponentManager<AABBComponent>&			CM_AABB()			{ return aabb; }
ComponentManager<CircleColliderComponent>&	CM_CircleCollider()	{ return circlecollider; }
ComponentManager<AudioComponent	>&			CM_Audio()			{ return audio; }
ComponentManager<LifetimeComponent>&		CM_Lifetime()		{ return lifetime; }
ComponentManager<HealthComponent>&			CM_Health()			{ return health; }
ComponentManager<TagComponent>&				CM_Tag()			{ return tag; }
ComponentManager<InputComponent>&			CM_Input()			{ return input; }

#pragma endregion COMPONENT
