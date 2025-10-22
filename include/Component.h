/* ========================================================
 * @file		Component.h
 * @author		Tan Jun Quan		| t.junquan@digipen.edu
 * @co-author	Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @date		29 SEP 2025
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
};
// BASE END

// GRAPHICS
// COMP_2
//struct SpriteComponent {
//	// Texture* tex;	// texture file (to check w reg/kerv)
//	unsigned int textureID = 0;   // OpenGL texture handle
//	Vec2 size	{ 50.0f, 50.0f }; // default size
//	Vec3 color	{ 1.0f, 1.0f, 1.0f}; // white (no tint)
//};

// COMP_3
//struct AnimatorComponent {
//	unsigned int total_cols, total_rows;
//	unsigned int curr_col, curr_row;
//
//	std::string state;
//};
// GRAPHICS END

// PHYSICS
//struct PhysicsComponent {
//	Vec2 vel	{ 0.f,0.f };
//	Vec2 accel	{ 0.f,0.f };
//	//bool collide (maybe can use bitwise checking to see WHAT things it will collide with? to consider)
//	//aabb hitbox;
//	bool active { false };
//	float width{ 0.f };
//	float height{ 0.f };
//};


struct AudioComponent {
	// Audiofile audio;
};

struct LifetimeComponent {
	float time;
};

struct HealthComponent {
	float currhp;
	float maxhp;
};

enum C_TagID {
	T_PLAYER,
	T_ENEMY,
	T_PROJECTILE,
	T_WALL,
	T_INSTANCING
};

struct TagComponent {
	C_TagID tag = T_PLAYER;
};

enum ControlScheme {
	CS_WASD,
	CS_IJKL,
	CS_ARROWS
};

struct InputComponent {
	ControlScheme scheme = CS_WASD;
};

enum ShapeType {
	SHAPE_RECTANGLE,
	SHAPE_CIRCLE
};

struct ShapeComponent {
	ShapeType shape = SHAPE_RECTANGLE;
	Vec2 dimensions = { 1.f, 1.f };
	Vec3 color = { 1.0f, 1.0f, 1.0f };
};

enum Element {
	ELEMENT_NONE,
	ELEMENT_FIRE,
	ELEMENT_ICE,
	ELEMENT_WIND,
	ELEMENT_LIGHTNING
};

struct ElementComponent {
	Element element = ELEMENT_NONE;
};