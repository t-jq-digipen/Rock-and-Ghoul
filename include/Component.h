/* ========================================================
 * @file		Component.h
 * @author		Tan Jun Quan		| t.junquan@digipen.edu
 * @co-author	Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author	Yasmin Tazi			| yasmin.t@digipen.edu
 * @date		19 NOV 2025
 *
 * @brief	Declaration of ECS component type IDs and minor
 *			components found within
 *
 * This file declares component type IDs (e.g CT_Sprite)
 * used for calling the different component managers within
 * registry (Registry.cpp)
 * 
 * More complex components will be declared and defined in
 * their own header and source files. Below is a list of
 * components sorted by Major and Minor:
 * 
 * MAJOR COMPONENTS:
 * - Physics		| Physics.cpp + .h	| Velocity, damping, forces
 * - Sprite			| Sprite.cpp + .h	| Image component
 * - Animator		| Animator.cpp + .h | Spritesheet management with relation to states.
 * 
 * MINOR COMPONENTS:
 * - Transform		|  Position, Rotation, Scale, Positional changes
 * - Audio			|  Audio (Pending, may be promoted to MAJOR)
 * - Lifetime		|  Lifetime, for timers or expirations
 * - Health			|  Current and max health
 * - Tag			|  Tag with common object types for easy tracking
 * - Input			|  Input component
 * - Shape			|  Basic shape component (for primitive shapes without sprites)
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

// Declare component structs and IDs i guess idk im sleepy
	
#pragma once
#include <bitset>
#include <list>
#include "Remath.h"
#include "Render.h"
#include "Collision.h"
#include "Entity.h"

#include "DataDefs.h"
#include <nlohmann/json.hpp>

#include "Registry.h"
#include "RenderComponent.h"


// System notes:


// Define component types using enum
enum ComponentTypeID
{
	CT_None = 0,
	// Basic
	CT_Transform,		// 1	| Position, Rotation, Scale.
							  
	// Graphics				  
	CT_Sprite,			// 2	| Texture or frame
	CT_Animator,		// 3	| Handles animations and state
						  
	// Physics			  
	CT_Physics,			// 4	| Velocity, acceleration
						  
	// Audio			  
	CT_Audio,			// 7	| Audio...
							  
	// Timer				  
	CT_Lifetime,		// 8	| Lifetime for timers or expirations

	// Game character specifics
	CT_Health,			// 9	| Health, can be used for player, enemies, breakable objects, etc.
	// Tag					  
	CT_Tag,				// 10	| Tag enumeration (should i use std::string? idk, think again another time)
							  
	CT_Input,

	CT_Shape,			//		| Shape enumeration for non-sprite drawing

	CT_Element
};


// basically for faster reading, consider implementing later
using ComponentMask = std::bitset<32>;


// BASE
// COMP_1
struct TransformComponent {
	// should these be vec3? i forget LOL
	Vec2 pos	{ 0.0f, 0.0f };
	Vec2 scale	{ 1.0f, 1.0f };
	Vec2 Prevpos;
	float rot{0.0f};

	std::string Serialize() const {
		// does serialization things here
		json j = {
			{"posX", pos.x},
			{"posY", pos.y},
			{"scaleX", scale.x},
			{"scaleY", scale.y},
			{"rot", rot}
		};
		return j.dump();
	}

	static TransformComponent DeSerialize(Entity eID, const std::string& data)  {
		(void)eID;
		// does deserialization things here
		// check if empty string first, otherwise error
		if (data.empty()) {
			return TransformComponent();
		}
		json j = json::parse(data);
		TransformComponent temp;
		temp.pos.x = j["posX"];
		temp.pos.y = j["posY"];
		temp.scale.x = j["scaleX"];
		temp.scale.y = j["scaleY"];
		temp.rot = j.value("rot", 0.0f);
		return temp;
	}


	Editor::ComponentData EditorData() {

		Editor::ComponentData ret("Transform");
		ret.Add_Data("posX",		{ Editor::Widget_Float{ &pos.x , true, 0.f, 1920.f } });
		ret.Add_Data("posY",		{ Editor::Widget_Float{ &pos.y , true, 0.f, 1080.f } });
		ret.Add_Data("scaleX",		{ Editor::Widget_Float{ &scale.x , true, 0.1f, 50.f } });
		ret.Add_Data("scaleY",		{ Editor::Widget_Float{ &scale.y , true, 0.1f, 50.f } });
		ret.Add_Data("Rotation",	{ Editor::Widget_Float{ &rot , true, -360.f, 360.f } });

		return ret;
	}
};
// BASE END

//struct AudioComponent {
//	// Audiofile audio;
//	std::string Serialize() const {
//		json j = {
//			{"",""}
//		};
//		return j.dump();
//	}
//
//};

struct LifetimeComponent {
	float age{};
	float lifetime{};

	std::string Serialize() const {
		json j = {
			{"age", age},
			{"lifetime", lifetime}
		};
		return j.dump();
	}

	static LifetimeComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return LifetimeComponent();
		}
		json j = json::parse(data);
		LifetimeComponent temp;
		temp.age = j.value("age", 0.0f);
		temp.lifetime = j.value("lifetime", temp.age);
		return temp;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Lifetime");
		ret.Add_Data("lifetime", { Editor::Widget_Float{&lifetime, true, 0.f, 100.f} });

		return ret;

	}
};

struct HealthComponent {
	float currhp;
	float maxhp;

	std::string Serialize() const {
		json j = {
			{"currentHP", currhp},
			{"maxHP", maxhp}
		};
		return j.dump();
	}

	static HealthComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return HealthComponent();
		}
		json j = json::parse(data);
		HealthComponent temp;
		temp.currhp = j.value("currentHP", 100.0f);
		temp.maxhp = j.value("maxHP", 100.0f);
		return temp;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Health");
		ret.Add_Data("currentHP",	Editor::Widget_Float{&currhp, true, 0.f, maxhp} );
		ret.Add_Data("maxHP",		Editor::Widget_Float{&maxhp, true, 0.f, 100.f} );

		return ret;
	}
};

enum C_TagID {
	T_PLAYER,
	T_ENEMY,
	T_PROJECTILE,
	T_WALL,
	T_INSTANCING,
	T_BATCH,
	T_TEST,
	T_OBJECT
};

struct TagComponent {
	C_TagID tag = T_PLAYER;

	std::string Serialize() const {
		json j = {
			{"tagID", static_cast<int>(tag)}
		};
		return j.dump();
	}

	static TagComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return TagComponent();
		}
		json j = json::parse(data);
		return TagComponent(static_cast<C_TagID>(static_cast<int>(j["tagID"])));
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Tag");
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&tag);
		dropdown.Add_Option("Player", static_cast<unsigned int>(T_PLAYER));
		dropdown.Add_Option("Enemy", static_cast<unsigned int>(T_ENEMY));
		dropdown.Add_Option("Projectile", static_cast<unsigned int>(T_PROJECTILE));
		dropdown.Add_Option("Wall", static_cast<unsigned int>(T_WALL));
		dropdown.Add_Option("Instancing", static_cast<unsigned int>(T_INSTANCING));
		dropdown.Add_Option("Test", static_cast<unsigned int>(T_TEST));
		dropdown.Add_Option("Object", static_cast<unsigned int>(T_OBJECT));

		ret.Add_Data("Tag",  dropdown );

		return ret;
	}
};

/**
 * @brief Collider component for collision detection and layer filtering
 *
 * Separated from PhysicsComponent to allow:
 * - Collision without physics (triggers, static objects)
 * - Physics without collision (ghost particles, visual effects)
 */
struct ColliderComponent {
	// Collision geometry
	AABB aabbhitbox;
	Circle circlehitbox;

	// Collision filtering (names for serialization, bits for runtime)
	//std::string layerName = "";                  // Layer name (for save/load)
	//std::vector<std::string> maskNames = {};     // Mask names (for save/load)
	uint32_t layerBit = 0;                       // Runtime layer bit
	// uint32_t maskBit = 0;                        // Runtime mask bit
	bool isTrigger = false;                      // If true, generate event but skip physics response
	std::string triggerEvent = "";               // Event name to publish on trigger collision

	// Collision state
	bool isActive = true;    // If false, skip collision checks entirely

	ColliderComponent()
		: aabbhitbox{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}}
		, circlehitbox{{0.f, 0.f}, 0.f}
		, layerBit(0)
//		, maskBit(0)
		, isTrigger(false)
		, isActive(true)
	{}

	ColliderComponent(AABB const& aabb, Circle const& circle, bool active = true)
		: aabbhitbox(aabb)
		, circlehitbox(circle)
		, layerBit(0)
//		, maskBit(0)
		, isTrigger(false)
		, isActive(active)
	{}

	// Hitbox updates
	void UpdateHitboxes(Vec2 const& pos) {
		Update_AABB(aabbhitbox, pos);
		circlehitbox.pos = pos;
	}

	// Layer management (implementations in Component.cpp or inline)
	void SetLayer(uint32_t bit) { layerBit = bit; }
	void SetTrigger(bool trigger) { isTrigger = trigger; }
	void SetActive(bool active) { isActive = active; }

	uint32_t GetLayer() const { return layerBit; }
	//uint32_t GetMask() const { return maskBit; }
	bool IsTrigger() const { return isTrigger; }
	bool IsActive() const { return isActive; }

	const AABB& Get_AABB() const { return aabbhitbox; }
	const Circle& Get_Circle() const { return circlehitbox; }
	AABB& Get_AABB() { return aabbhitbox; }
	Circle& Get_Circle() { return circlehitbox; }

	void Set_AABB(AABB const& newAABB) { aabbhitbox = newAABB; }
	void Set_Circle(Circle const& newCircle) { circlehitbox = newCircle; }

	// String-based layer setters (need CollisionManager)
	void SetLayer(const std::string& name);
	//void SetMask(const std::vector<std::string>& names);
	//void SetMask(uint32_t mask);  // Overload for editor to update both maskBit and maskNames

	std::string Serialize() const;
	static ColliderComponent DeSerialize(Entity eID, const std::string& data);

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Collider");
		ret.Add_Data("AABB Min X", Editor::Widget_Float{&aabbhitbox.min.x});
		ret.Add_Data("AABB Min Y", Editor::Widget_Float{&aabbhitbox.min.y});
		ret.Add_Data("AABB Dim X", Editor::Widget_Float{&aabbhitbox.dim.x});
		ret.Add_Data("AABB Dim Y", Editor::Widget_Float{&aabbhitbox.dim.y});
		ret.Add_Data("Circle Radius", Editor::Widget_Float{&circlehitbox.rad});
		ret.Add_Data("Is Trigger", Editor::Widget_Checkmark{&isTrigger});
		ret.Add_Data("Is Active", Editor::Widget_Checkmark{&isActive});
		return ret;
	}
};


struct RenderLayerComponent {
	RenderLayer layer = LAYER_GAMEOBJECT;

	std::string Serialize() const {
		json j = { {"layer", static_cast<int>(layer)} }; //use layer in json
		return j.dump();
	}

	static RenderLayerComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) return RenderLayerComponent();
		json j = json::parse(data);
		RenderLayerComponent temp;
		temp.layer = static_cast<RenderLayer>(j.value("layer", 1)); // default = GAMEOBJECT
		return temp;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("RenderLayer");
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&layer);
		dropdown.Add_Option("Background", LAYER_BACKGROUND);
		dropdown.Add_Option("GameObject", LAYER_GAMEOBJECT);
		dropdown.Add_Option("Foreground", LAYER_FOREGROUND);
		dropdown.Add_Option("UI", LAYER_UI);
		ret.Add_Data("Layer", dropdown);
		return ret;
	}
};

enum ControlScheme {
	CS_WASD,
	CS_IJKL,
	CS_ARROWS
};

struct InputComponent {
	ControlScheme scheme = CS_WASD;

	std::string Serialize() const {
		json j = {
			{"scheme", static_cast<int>(scheme)}
		};
		return j.dump();
	}

	static InputComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return InputComponent();
		}
		json j = json::parse(data);
		InputComponent temp;
		temp.scheme = static_cast<ControlScheme>(static_cast<int>(j["scheme"]));
		return temp;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Input");
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&scheme);
		dropdown.Add_Option("WASD", static_cast<unsigned int>(CS_WASD));
		dropdown.Add_Option("IJKL", static_cast<unsigned int>(CS_IJKL));
		dropdown.Add_Option("Arrows", static_cast<unsigned int>(CS_ARROWS));

		ret.Add_Data("Scheme",  dropdown );

		return ret;
	}
};


//enum Element {
//	ELEMENT_NONE,
//	ELEMENT_FIRE,
//	ELEMENT_ICE,
//	ELEMENT_WIND,
//	ELEMENT_LIGHTNING
//};
//
//struct ElementComponent {
//	Element element = ELEMENT_NONE;
//};