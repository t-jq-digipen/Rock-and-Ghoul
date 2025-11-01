/* ========================================================
 * @file		Component.h
 * @author		Tan Jun Quan		| t.junquan@digipen.edu
 * @co-author	Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @date		26 OCT 2025
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
	float time;

	std::string Serialize() const {
		json j = {
			{"time", time}
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
		temp.time = j.value("time", 0.0f);
		return temp;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Lifetime");
		ret.Add_Data("time", { Editor::Widget_Float{&time, true, 0.f, 100.f} });

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
	T_BATCH
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

		ret.Add_Data("Tag",  dropdown );

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

enum ShapeType {
	SHAPE_RECTANGLE,
	SHAPE_CIRCLE
};

struct ShapeComponent {
	ShapeType shape = SHAPE_RECTANGLE;
	Vec2 dimensions = { 1.f, 1.f };
	Vec3 color = { 1.0f, 1.0f, 1.0f };

	std::string Serialize() const {
		json j = {
			{"shapeType", static_cast<int>(shape)},
			{"dimX", dimensions.x},
			{"dimY", dimensions.y},
			{"colorX", color.x},
			{"colorY", color.y},
			{"colorZ", color.z}
		};
		return j.dump();
	}

	static ShapeComponent DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return ShapeComponent();
		}
		json j = json::parse(data);

		ShapeComponent s;

		s.shape = static_cast<ShapeType>(static_cast<int>(j["shapeType"]));
		s.dimensions.x = j["dimX"];
		s.dimensions.y = j["dimY"];
		s.color.x = j["colorX"];
		s.color.y = j["colorY"];
		s.color.z = j["colorZ"];

		return s;
	}

	Editor::ComponentData EditorData() {
		Editor::ComponentData ret("Shape");
		Editor::Widget_Dropdown dropdown;
		dropdown.ref = reinterpret_cast<unsigned int*>(&shape);
		dropdown.Add_Option("Rectangle", static_cast<unsigned int>(SHAPE_RECTANGLE));
		dropdown.Add_Option("Circle", static_cast<unsigned int>(SHAPE_CIRCLE));
		ret.Add_Data("Shape", dropdown );

		ret.Add_Data("Width",		  Editor::Widget_Float{&dimensions.x, true, 0.f, 100.f});
		ret.Add_Data("Height",		  Editor::Widget_Float{&dimensions.y, true, 0.f, 100.f});

		ret.Add_Data("Color:Red",	 Editor::Widget_Float{&color.x, true, 0.f, 1.f});
		ret.Add_Data("Color:Green",  Editor::Widget_Float{&color.y, true, 0.f, 1.f});
		ret.Add_Data("Color:Blue",   Editor::Widget_Float{&color.z, true, 0.f, 1.f});

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