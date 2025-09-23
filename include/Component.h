
// Declare component structs and IDs i guess idk im sleepy
	
#pragma once
#include <bitset>

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

};


// basically for faster reading, consider implementing later
using ComponentMask = std::bitset<32>;

class BaseComponent 
{

};

// BASE
// COMP_1
struct TransformComponent {
	// should these be vec3? i forget LOL
	Vec2 pos;	// Position
	Vec2 rot;	// Rotation
	Vec2 scale;	// Scale
};
// BASE END

// GRAPHICS
// COMP_2
struct SpriteComponent {
	// Texture* tex;	// texture file (to check w reg/kerv)
	int frame;			// frame to display
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
	Vec2 accel;
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