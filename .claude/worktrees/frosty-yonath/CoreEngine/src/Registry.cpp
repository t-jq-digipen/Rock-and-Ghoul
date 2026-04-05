/* ========================================================
 * @file		Registry.cpp
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi  | yasmin.t@digipen.edu

 * @date		01 OCT 2025
 *
 * @brief	Registry for ECS management
 *
 * This registry functions as the connection between the
 * entities and components, and is the medium for the systems
 * to interact with entitites and their components.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

// IMPORTANT!!! DO NOT AMEND ANYTHING IN THIS FILES WITHOUT ASKING ME!!!!!!!
#include "Registry.h"
// Tis shid the glue between the E the C and the S so like E + C + S = registry then u could call it registry == ECS lololol i need sleep its 5am



//// COMPONENT MANAGERS
//// ================================================================================
//// CT_Transform
//ComponentManager<TransformComponent>			transform;
//
//// Graphics
//// CT_Sprite
//ComponentManager<Sprite::SpriteComponent>		sprite;
//
//// CT_Animator
//ComponentManager<Animator::AnimatorComponent>	animator;
//
//// Physics
//// CT_Physics
//ComponentManager<Physics::PhysicsComponent>		physics;
//
//
//// Audio
//// CT_Audio
//ComponentManager<AudioComponent>				audio;
//
//// Timer
//// CT_Lifetime
//ComponentManager<LifetimeComponent>				lifetime;
//
//// Stats
//// CT_Health
//ComponentManager<HealthComponent>				health;
//
//// Tag
//// CT_Tag
//ComponentManager<TagComponent>					tag;
//
//// Input
//// CT_Input
//ComponentManager<InputComponent>				input;
//
//// Shape
//// CT_Shape
//ComponentManager<ShapeComponent>				shape;
//
//// Element
//// CT_Elemnt
//ComponentManager<ElementComponent>				element;
//
//
//// Entity manager (stores a REFERENCE to the singleton entity manager ^^)
//// ================================================================================
//EntityManager& entityManager = EntityManager::get();
//
//// ENTITY REGION
//#pragma region ENTITY
//
//Entity Create_Entity()
//{
//	std::cout << "Entity creating!\n";
//	return entityManager.createEntity();
//}
//
//
//void Remove_Entity(Entity eID)
//{
//	// remove all components!
//	// Remove_Components(eID, { CT_Transform, CT_Sprite, CT_Animator, CT_Physics, CT_Audio, CT_Lifetime, CT_Health, CT_Tag, CT_Input, CT_Shape, CT_Element });
//
//	// remove entity ID!
//	entityManager.destroyEntity(eID);
//}
//#pragma endregion ENTITY
//
//
//// COMPONENT REGION
//#pragma region COMPONENT
///*
//COMPONENTS LIST:
//	1 | CT_Transform	| For position, rotate, and scale data
//	2 | CT_Sprite		| Sprite texture data, sprite frame
//	3 | CT_Animator		| Animation handler
//	4 | CT_Physics		| Collision, hitbox
//	5 | CT_Audio		| Audio
//	6 | CT_Lifetime		| Lifetime
//	7 | CT_Health		| Health/Maxhealth
//	8 | CT_Tag			| Tag
//*/
//
//// ADD
//#pragma region ADD COMPONENT
//
//void Add_Component(Entity eID, TransformComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | TRANSFORM	| eID: " << eID << "\n";
//	transform.addComponent(eID, comp);
//}
//
//void Add_Component(Entity eID, Sprite::SpriteComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | SPRITE		| eID: " << eID << "\n";
//	sprite.addComponent(eID, comp);
//}
//
//void Add_Component(Entity eID, Animator::AnimatorComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | ANIM		| eID: " << eID << "\n";
//	animator.addComponent(eID, comp);
//}
//
//void Add_Component(Entity eID, Physics::PhysicsComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | PHYSICS		| eID: " << eID << "\n";
//	physics.addComponent(eID, comp);
//}
//

//void Add_Component(Entity eID, AudioComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | AUDIO		| eID: " << eID << "\n";
//	audio.addComponent(eID, comp);
//
//void Add_Component(Entity eID, LifetimeComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | LIFETIME	| eID: " << eID << "\n";
//	lifetime.addComponent(eID, comp);
//}
//
//void Add_Component(Entity eID, HealthComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | HEALTH		| eID: " << eID << "\n";
//	health.addComponent(eID, comp);
//}
//
//void Add_Component(Entity eID, TagComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | TAG	        | eID: " << eID << "\n";
//	tag.addComponent(eID, comp);
//}
//void Add_Component(Entity eID, InputComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | INPUT       | eID: " << eID << "\n";
//	input.addComponent(eID, comp);
//}
//void Add_Component(Entity eID, ShapeComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | SHAPE       | eID: " << eID << "\n";
//	shape.addComponent(eID, comp);
//}
//void Add_Component(Entity eID, ElementComponent const& comp)
//{
//	std::cout << "REGISTRY | ADDING COMPONENT | ELEMENT	    | eID: " << eID << "\n";
//	element.addComponent(eID, comp);
//}
//
//
//#pragma endregion
//
//
//
//
//// REMOVE
//
//void Remove_Component(Entity eID, ComponentTypeID const& cID)
//{
//	switch (cID)
//	{
//	case (CT_None):
//	{
//		break;
//	}
//	case (CT_Transform):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | TRANSFORM		| eID: " << eID << "\n";
//		transform.removeComponent(eID);
//	}
//	break;	// CT_Transform
//
//	case (CT_Sprite):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | SPRITE		| eID: " << eID << "\n";
//		sprite.removeComponent(eID);
//	}
//	break;	// CT_Sprite
//
//	case (CT_Animator):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | ANIM			| eID: " << eID << "\n";
//		animator.removeComponent(eID);
//	}
//	break;	// CT_Animator
//
//	case (CT_Physics):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | PHYSICS		| eID: " << eID << "\n";
//		physics.removeComponent(eID);
//	}
//	break;	// CT_Physics
//
//	case (CT_Audio):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | AUDIO			| eID: " << eID << "\n";
//		audio.removeComponent(eID);
//	}
//	break;	// CT_Audio
//
//	case (CT_Lifetime):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | LIFETIME		| eID: " << eID << "\n";
//		lifetime.removeComponent(eID);
//	}
//	break;	// CT_Lifetime
//
//	case (CT_Health):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | HEALTH		| eID: " << eID << "\n";
//		health.removeComponent(eID);
//	}
//	break;	// CT_Health
//
//	case (CT_Tag):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | TAG			| eID: " << eID << "\n";
//		tag.removeComponent(eID);
//	}
//	break;	// CT_Tag
//
//	case (CT_Input):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | INPUT			| eID: " << eID << "\n";
//		input.removeComponent(eID);
//	}
//	break;	// CT_Tag
//
//	case (CT_Shape):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | SHAPE			| eID: " << eID << "\n";
//		shape.removeComponent(eID);
//	}
//	break;	// CT_Shape
//
//	case (CT_Element):
//	{
//		std::cout << "REGISTRY | REMOVING COMPONENT | ELEMENT       | eID: " << eID << "\n";
//		element.removeComponent(eID);
//	}
//	break;	// CT_Element
//	}
//}
//
//
//void Remove_Components(Entity eID, std::initializer_list<ComponentTypeID> cIDlist)
//{
//	for (ComponentTypeID cID : cIDlist)
//	{ Remove_Component(eID, cID); }
//}
//
//bool Has_Component(Entity eID, ComponentTypeID const& cID)
//{
//
//	switch (cID)
//	{
//	case (CT_None):
//	{
//		return false;
//		break;
//	}
//	case (CT_Transform):
//	{ return transform.hasComponent(eID); }
//	break;	// CT_Transform
//
//	case (CT_Sprite):
//	{ return sprite.hasComponent(eID); }
//	break;	// CT_Sprite
//
//	case (CT_Animator):
//	{ return animator.hasComponent(eID); }
//	break;	// CT_Animator
//
//	case (CT_Physics):
//	{ return physics.hasComponent(eID); }
//	break;	// CT_Physics
//
//	case (CT_Audio):
//	{ return audio.hasComponent(eID); }
//	break;	// CT_Audio
//
//	case (CT_Lifetime):
//	{ return lifetime.hasComponent(eID); }
//	break;	// CT_Lifetime
//
//	case (CT_Health):
//	{ return health.hasComponent(eID); }
//	break;	// CT_Health
//
//	case (CT_Tag):
//	{ return tag.hasComponent(eID); }
//	break;	// CT_Tag
//
//	case (CT_Input):
//	{ return input.hasComponent(eID); }
//	break;	// CT_Tag
//
//	case (CT_Shape):
//	{ return shape.hasComponent(eID); }
//	break;	// CT_Tag
//
//	case (CT_Element):
//	{ return element.hasComponent(eID); }
//	break;	// CT_Element
//
//	default:
//		return false;
//	}
//}
//
//
//// GET
//
//// Gets the component of the entity from the specified type
//TransformComponent&				Get_TransformComponent(Entity eID)		{ return transform.getComponent(eID); }
//
//Sprite::SpriteComponent&		Get_SpriteComponent(Entity eID)			{ return sprite.getComponent(eID); }
//
//Animator::AnimatorComponent&	Get_AnimatorComponent(Entity eID)		{ return animator.getComponent(eID); }
//
//Physics::PhysicsComponent&		Get_PhysicsComponent(Entity eID)		{ return physics.getComponent(eID); }
//
//AudioComponent&					Get_AudioComponent(Entity eID)			{ return audio.getComponent(eID); }
//
//LifetimeComponent&				Get_LifetimeComponent(Entity eID)		{ return lifetime.getComponent(eID); }
//
//HealthComponent&				Get_HealthComponent(Entity eID)			{ return health.getComponent(eID); }
//
//TagComponent&					Get_TagComponent(Entity eID)			{ return tag.getComponent(eID); }
//
//InputComponent&					Get_InputComponent(Entity eID)			{ return input.getComponent(eID); }
//
//ShapeComponent&					Get_ShapeComponent(Entity eID)			{ return shape.getComponent(eID); }
//

//
//
//// Manager accessors
//ComponentManager<TransformComponent>&			CM_Transform()		{ return transform; }
//ComponentManager<Sprite::SpriteComponent>&		CM_Sprite()			{ return sprite; }
//ComponentManager<Animator::AnimatorComponent>&	CM_Animator()		{ return animator; }
//ComponentManager<Physics::PhysicsComponent>&	CM_Physics()		{ return physics; }
//ComponentManager<LifetimeComponent>&			CM_Lifetime()		{ return lifetime; }
//ComponentManager<HealthComponent>&				CM_Health()			{ return health; }
//ComponentManager<TagComponent>&					CM_Tag()			{ return tag; }
//ComponentManager<InputComponent>&				CM_Input()			{ return input; }
//ComponentManager<ShapeComponent>&				CM_Shape()			{ return shape; }
//#pragma endregion COMPONENT
//
//std::unordered_map<Entity, std::string>& GetEntityNameMap() {
//	static std::unordered_map<Entity, std::string> entityNames;
//	return entityNames;
//}
//
//void SetEntityName(Entity eID, const std::string& name) {
//	GetEntityNameMap()[eID] = name;
//}
//
//std::string GetEntityName(Entity eID) {
//	auto& nameMap = GetEntityNameMap();
//	auto it = nameMap.find(eID);
//	if (it != nameMap.end()) {
//		return it->second;
//	}
//	return "Entity_" + std::to_string(eID);
//}

std::vector<Editor::ComponentData> Registry::Get_EntityData(Entity eID) {
	std::vector<Editor::ComponentData> ret;
	for (auto& [type, manager] : componentManagers)
	{
		if (manager->hasComponent(eID))
		{ ret.push_back(manager->GetComponentEditorData(eID)); }
	}

	return ret;
}