
// Declare component structs and IDs i guess idk im sleepy
	
#pragma once
#include <bitset>


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

};


// basically for faster reading
using ComponentMask = std::bitset<32>;


class BaseComponent 
{

};

// BASE
// COMP_1
struct Transform {
	Vec2 pos;
	Vec2 rot;
	Vec2 scale;
};
// BASE END

// GRAPHICS
// COMP_2
struct Sprite {
	// Texture* tex;	// texture file (to check w reg/kerv)
	int frame;			// frame to display
};

// COMP_3
struct Animator {
	std::string State;	// probably change to an enum for animstate later on
	float time;			// for anim timers
};
// GRAPHICS END

// PHYSICS
struct Physics {
	Vec2 vel;
	//aabb hitbox;
};