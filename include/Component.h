
// Declare component structs and IDs i guess idk im sleepy
	
#pragma once
#include <bitset>
#include <list>
#include "Remath.h"
#include "Render.h"

// System notes:
/*
* Physics system
* -> READS physics component
* -> UPDATES physics component
* -> WRITES into transform component
* 
* Render system
* -> READS transform component
*/

// Define component types using enum
enum ComponentTypeID
{
	CT_None = 0,
	// Basic
	CT_Transform,	// 1 | Position, Rotation, Scale.

	// Graphics
	CT_Sprite,		// 2 | Texture or frame
	CT_Animator,	// 3 | Handles animations and state

	// Physics
	CT_Physics,		// 4 | Velocity, acceleration, collision, hitboxes

	// Audio
	CT_Audio,		// 5 | Audio...

	// Timer
	CT_Lifetime,	// 6 | Lifetime for timers or expirations

	// Game character specifics
	CT_Health,		// 7 | Health, can be used for player, enemies, breakable objects, etc.
	CT_GhoulHealth,
	// Tag
	CT_Tag,			// 8 | Tag enumeration (should i use std::string? idk, think again another time)
	CT_BossGhoul
};


// basically for faster reading, consider implementing later
using ComponentMask = std::bitset<32>;

// BASE
// COMP_1
struct TransformComponent {
	// should these be vec3? i forget LOL
	Vec2 pos{ 0.0f, 0.0f };
	Vec2 scale{ 1.0f, 1.0f };
};
// BASE END

// GRAPHICS
// COMP_2
struct SpriteComponent {
	// Texture* tex;	// texture file (to check w reg/kerv)
	unsigned int textureID = 0;   // OpenGL texture handle
	Vec2 size{ 50.0f, 50.0f }; // default size
	Vec3 color{ 1.0f, 1.0f, 1.0f}; // white (no tint)
};

// COMP_3
struct AnimatorComponent {
	std::string State;	// probably change to an enum for animstate later on? depending on animation states available...
	float time;			// for anim timers
};
// GRAPHICS END

// PHYSICS
struct PhysicsComponent {
	Vec2 vel;
	//Vec2 accel;
	//bool collide (maybe can use bitwise checking to see WHAT things it will collide with? to consider)
	//aabb hitbox;
};

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
};

struct TagComponent {
	C_TagID tag;
};

enum ControlScheme {
	CS_WASD,
	CS_IJKL,
	CS_ARROWS
};

struct InputComponent {
	ControlScheme scheme;
};

enum GhoulState {
	GHOUL_START,
	GHOUL_ATTACKING,
	GHOUL_DEAD
};

struct BossGhoulComponent {
	float currhp = 0.f;
	float maxhp = 100.f;
	float speedX = 0.f;
	float speedY = 0.f;
	int moveDir = 0;
	GhoulState state = GHOUL_START;
};
