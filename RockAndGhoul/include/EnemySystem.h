/* ========================================================
 * @file	    EnemySystem.h
 * @author	    Manfred Sim         | manfred.sim@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @date	    01 NOV 2025
 *
 * @brief	Enemy System for handling enemy behaviour.
 *
 * This file declares an enemy system for the core engine,
 * determining enemy logic with scripts and enemy states.
 * 
 * Enemy States:
 * - E_IDLE : Enemy is idle and stationary.
 * - E_ROAMING : Enemy is roaming around its starting position.
 * - E_DETECTING : Enemy is checking if player is nearby. (NP)
 * - E_CHASING : Enemy is chasing after the player.
 * - E_ATTACKING : Enemy is targeting the player with attacks. (NP)
 * - E_COOLDOWN : Enemy enters state of cooldown after executing an attack. (NP)
 * - E_STAGGERED : Enemy staggers when attacked. (NP)
 * - E_RETURNING : Enemy is returning to its starting position.
 * 
 * (NP) - Not implemented yet.
 *
 * Copyright ? 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "System.h"
#include "CombatSystem.h"

// Set enemy AI behavior
enum EnemyState : unsigned int
{
	E_IDLE,
	E_ROAMING,
	E_DETECTING,
	E_CHASING,
	E_ATTACKING,
	E_COOLDOWN,
	E_STAGGERED,
	E_RETURNING
};

bool HasBarrier(Entity eID);
bool BreakBarrier(Entity eID, Element element);

namespace EnemyAI {

	inline void Start(Entity eID);

	inline void Update(Entity eID, float dt);

	inline void End(Entity eID);
}

enum SpiritState : unsigned int {
	SPIRIT_IDLE,
	SPIRIT_CHASE,
	SPIRIT_MELEE,
	SPIRIT_ESCAPE,
	SPIRIT_RANGE,
	SPIRIT_HIT,
	SPIRIT_DEAD
};

namespace SpiritScript {
	inline void Start(Entity eID);
	inline void Update(Entity eID, float dt);
	inline void End(Entity eID);
}

enum ObstacleState : unsigned int {
	OBSTACLE_IDLE,
	OBSTACLE_SHOCKED,
	OBSTACLE_BURNING,
	OBSTACLE_EXPLODE,
	OBSTACLE_FROZEN,
	OBSTACLE_PUSHED,
	OBSTACLE_DESTROYED
};

namespace ObstacleScript {
	inline void Start(Entity eID);
	inline void Update(Entity eID, float dt);
	inline void End(Entity eID);
}

namespace GhoulScript {
	enum GhoulState : unsigned int {
		GHOUL_SPAWN,
		GHOUL_TRANSFORM,
		GHOUL_IDLE,
		GHOUL_MOVING,
		GHOUL_BARRIER,
		GHOUL_STAGGERED,
		GHOUL_MELEE,
		GHOUL_RANGE,
		GHOUL_DEAD
	};

	enum GhoulAction {

		GHOUL_ACTION_MELEE,
		GHOUL_ACTION_RANGE
	};
	

	inline void Start(Entity eID);
	inline void Update(Entity eID, float dt);
	inline void End(Entity eID);
}

class EnemySystem : public ISystem
{
private:
	size_t SceneSubscriberID{};
public:
	EnemySystem() {}
	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;
};
