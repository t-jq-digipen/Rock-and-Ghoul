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
void Add_Component(Entity eID, InputComponent const& comp);
void Add_Component(Entity eID, BossGhoulComponent const& comp);
// overloads for removal
void Remove_Component(Entity eID, ComponentTypeID cID);
void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist);

extern ComponentManager<TransformComponent> transform;
extern ComponentManager<SpriteComponent>    sprite;
extern ComponentManager<AnimatorComponent>  animator;
extern ComponentManager<PhysicsComponent>   physics;
extern ComponentManager<AudioComponent>     audio;
extern ComponentManager<LifetimeComponent>  lifetime;
extern ComponentManager<HealthComponent>    health;
extern ComponentManager<TagComponent>       tag;
extern ComponentManager<InputComponent>		input;
extern ComponentManager<BossGhoulComponent> bossghoul;

template<typename T>
bool HasComponent(Entity e);

template<typename T>
T& GetComponent(Entity e);

// Transform
template<>
inline bool HasComponent<TransformComponent>(Entity e) { return transform.hasComponent(e); }

template<>
inline TransformComponent& GetComponent<TransformComponent>(Entity e) { return transform.getComponent(e); }

// Physics
template<>
inline bool HasComponent<PhysicsComponent>(Entity e) { return physics.hasComponent(e); }

template<>
inline PhysicsComponent& GetComponent<PhysicsComponent>(Entity e) { return physics.getComponent(e); }

// Sprite
template<>
inline bool HasComponent<SpriteComponent>(Entity e) { return sprite.hasComponent(e); }

template<>
inline SpriteComponent& GetComponent<SpriteComponent>(Entity e) { return sprite.getComponent(e); }

// Tag
template<>
inline bool HasComponent<TagComponent>(Entity e) { return tag.hasComponent(e); }

template<>
inline TagComponent& GetComponent<TagComponent>(Entity e) { return tag.getComponent(e); }

// Input
template<>
inline bool HasComponent<InputComponent>(Entity e) { return input.hasComponent(e); }

template<>
inline InputComponent& GetComponent<InputComponent>(Entity e) { return input.getComponent(e); }

template<>
inline bool HasComponent<BossGhoulComponent>(Entity e) { return bossghoul.hasComponent(e); }

template<>
inline BossGhoulComponent& GetComponent<BossGhoulComponent>(Entity e) { return bossghoul.getComponent(e); }

