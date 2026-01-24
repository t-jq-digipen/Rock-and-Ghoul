/* ========================================================
 * @file	    EnemySystem.cpp
 * @author	    Manfred Sim         | manfred.sim@digipen.edu
 * @co-author	Yasmin Tazi			| yasmin.t@digipen.edu
 * @date	    24 NOV 2025
 *
 * @brief	Enemy System for handling enemy behaviour.
 *
 * This file implements the logic of an enemy entity based on
 * the attached script and switches between enemy states
 * accordingly.
 * 
 * Current Implemented Enemy Logic:
 * - Enemy enters E_ROAMING state upon startup, moving up and
 * down around its starting position.
 * - If player enters a detection radius, enemy enters E_CHASING
 * state and gives chase after the player.
 * - If player manages to get far enough from the enemy, enemy
 * enters E_RETURNING state and quickly returns to its starting
 * position.
 * - While in E_RETURNING, enemy will return to E_ROAMING state
 * once it reaches its starting position. Otherwise if player
 * enters the enemy's detection radius, enemy will return to
 * E_CHASING state again and continue to chase after the player.
 * - Enemy subscribes to ProjectileHitEnemy trigger event, which
  * fires when a projectile collides with an enemy.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */


#include "EnemySystem.h"
#include "MovementSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Remath.h"
#include "BehaviourFCT.h"
#include "LogicSystem.h"
#include "SceneManager.h"
#include "Enemy.h"
#include <unordered_map>

static constexpr float DETECT_RADIUS = 300.f;
static constexpr float LOSE_RADIUS = 360.f;
static constexpr float STOP_DISTANCE = 8.f;

static constexpr float ROAM_SINE_SPEED = 2.f;
static constexpr float ROAM_SPEED_MAX = 200.f;
static constexpr float ROAM_AMPLITUDE = 100.f;
static constexpr float ROAM_EPS = 0.5f;

static constexpr float CHASE_SPEED = 300.f;
static constexpr float RETURN_SPEED = 600.f;

static constexpr float MELEE_RANGE = 60.f;
static constexpr float MELEE_COOLDOWN = 0.8f;
static constexpr float MELEE_WINDUP = 0.15f;

static constexpr float SHOOT_RANGE = 240.f;
static constexpr float SHOOT_COOLDOWN = 1.2f;
static constexpr float PROJECTILE_SPEED = 600.f;

enum class AttackMode : unsigned int
{
	MELEE_ONLY,
	PROJECTILE_ONLY
};

enum class AIState : unsigned int
{
	ROAMING,
	CHASING,
	ATTACKING,
	COOLDOWN,
	RETURNING
};

struct EnemyRuntime
{
	AIState state{ AIState::ROAMING };

	AttackMode attackMode{ AttackMode::PROJECTILE_ONLY };

	// base position
	Vec2 basePos{ 0.f, 0.f };
	bool baseSet{ false };

	// roam time
	float t{ 0.f };

	// cooldown
	float cooldown{ 0.f };

	// ranged
	float shootTimer{ 0.f };
	int ammo{ 1 };

	float windup{ 0.f };
	bool hitQueued{ false };

	bool didAttack{ false };
};

static std::unordered_map<Entity, EnemyRuntime> g_ai;

static inline bool HasTransform(Entity e)
{
	return e != INVALID_ENTITY && Registry::Get().Has_Component<TransformComponent>(e);
}

static inline Vec2 GetPos(Entity e)
{
	auto& tr = Registry::Get().Get_Component<TransformComponent>(e);
	return { tr.pos.x, tr.pos.y };
}

static inline float DistSq(Vec2 a, Vec2 b)
{
	return RG_DistanceSquared(a, b);
}

static inline Movement::MovementComponent& GetMovement(Entity e)
{
	return Registry::Get().Get_Component<Movement::MovementComponent>(e);
}

static inline void ApplyVelocity(Entity e, Vec2 desiredDir, float speed, bool moving)
{
	auto& mv = GetMovement(e);
	mv.SetMoveType(Movement::MOVE_POSITION);

	if (!moving || RG_LengthSquared(desiredDir) <= 0.0001f)
	{
		mv.SetMovementDirection({ 0.f, 0.f });
		mv.SetMoveSpeed(0.f);
		mv.SetIsMoving(false);
		mv.SetInputDirection(Movement::MOVE_NONE);
		return;
	}

	float len2 = RG_LengthSquared(desiredDir);
	float inv = 1.f / RG_Sqrt(len2);
	Vec2 dir = { desiredDir.x * inv, desiredDir.y * inv };

	mv.SetInputDirection(Movement::MOVE_NONE);
	mv.SetMovementDirection(dir);
	mv.SetMoveSpeed(speed);
	mv.SetIsMoving(true);
}

static inline Entity FindPlayer()
{
	for (auto& [entity, comp] : Registry::Get().Get_ComponentMap<TagComponent>())
	{
		if (comp.tag == T_PLAYER && HasTransform(entity))
			return entity;
	}
	return INVALID_ENTITY;
}

static inline void ApplyMeleeDamage(Entity enemy, Entity target)
{
	(void)enemy; (void)target;
	std::cout << "[ENEMY] Melee hit target!\n";
}

static inline void SpawnEnemyProjectile(Entity enemy, Vec2 pos, Vec2 dir, float speed)
{
	(void)enemy; (void)pos; (void)dir; (void)speed;
	std::cout << "[ENEMY] Shot projectile!\n";
}

static inline AttackMode DecideAttackMode(Entity e)
{
	(void)e;
	return AttackMode::PROJECTILE_ONLY;
}

namespace EnemyAI
{
	inline void Start(Entity eID)
	{
		if (!HasTransform(eID)) return;
		if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID)) return;

		auto& st = g_ai[eID];
		st.state = AIState::ROAMING;
		st.t = 0.f;
		st.cooldown = 0.f;
		st.shootTimer = 0.f;
		st.ammo = 1;
		st.windup = 0.f;
		st.hitQueued = false;
		st.didAttack = false;

		st.attackMode = DecideAttackMode(eID);

		st.basePos = GetPos(eID);
		st.baseSet = true;

		ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);
	}

	inline void Update(Entity eID, float dt)
	{
		if (dt <= 0.f) return;
		if (!HasTransform(eID)) return;
		if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID)) return;

		auto it = g_ai.find(eID);
		if (it == g_ai.end())
		{
			Start(eID);
			it = g_ai.find(eID);
			if (it == g_ai.end()) return;
		}
		auto& st = it->second;

		// timers
		st.t += dt;
		st.cooldown = std::max(0.f, st.cooldown - dt);
		st.shootTimer = std::max(0.f, st.shootTimer - dt);
		st.windup = std::max(0.f, st.windup - dt);

		const Entity player = FindPlayer();
		const bool hasPlayer = HasTransform(player);

		const Vec2 selfPos = GetPos(eID);
		const Vec2 basePos = st.basePos;

		const float detectSq = DETECT_RADIUS * DETECT_RADIUS;
		const float loseSq = LOSE_RADIUS * LOSE_RADIUS;

		float dPlayerSq = std::numeric_limits<float>::infinity();
		Vec2 playerPos{ 0.f,0.f };
		if (hasPlayer)
		{
			playerPos = GetPos(player);
			dPlayerSq = DistSq(selfPos, playerPos);
		}

		// attack range
		const float atkRange = (st.attackMode == AttackMode::MELEE_ONLY) ? MELEE_RANGE : SHOOT_RANGE;
		const float atkSq = atkRange * atkRange;
		if (hasPlayer)
		{
			std::cout << "[DBG] e=" << (int)eID
				<< " state=" << (int)st.state
				<< " d=" << std::sqrt(dPlayerSq)
				<< " atk=" << std::sqrt(atkSq)
				<< " mode=" << (st.attackMode == AttackMode::MELEE_ONLY ? "MELEE" : "RANGED")
				<< "\n";
		}
		switch (st.state)
		{
		case AIState::ROAMING:
			if (hasPlayer && dPlayerSq <= detectSq) st.state = AIState::CHASING;
			break;

		case AIState::CHASING:
			if (!hasPlayer || dPlayerSq > loseSq)
			{
				st.state = AIState::RETURNING;
			}
			else if (dPlayerSq <= atkSq)
			{
				st.state = AIState::ATTACKING;
				st.windup = 0.f;
				st.hitQueued = false;
				st.didAttack = false;
			}
			break;

		case AIState::ATTACKING:
			if (!hasPlayer || dPlayerSq > loseSq)
			{
				st.state = AIState::RETURNING;
			}
			else if (dPlayerSq > atkSq)
			{
				st.state = AIState::CHASING;
				st.windup = 0.f;
				st.hitQueued = false;
				st.didAttack = false;
			}
			else
			{
				if (st.didAttack)
				{
					st.state = AIState::COOLDOWN;
				}
			}
			break;


		case AIState::COOLDOWN:
			if (st.cooldown <= 0.f)
			{
				if (st.attackMode == AttackMode::PROJECTILE_ONLY) st.ammo = 1;

				st.state = (hasPlayer && dPlayerSq <= atkSq) ? AIState::ATTACKING : AIState::CHASING;

				if (st.state == AIState::ATTACKING)
				{
					st.windup = 0.f;
					st.hitQueued = false;
				}
			}
			break;

		case AIState::RETURNING:
		{
			const float dBaseSq = DistSq(selfPos, basePos);
			if (hasPlayer && dPlayerSq <= detectSq) st.state = AIState::CHASING;
			else if (dBaseSq <= ROAM_EPS * ROAM_EPS) st.state = AIState::ROAMING;
			break;
		}

		default:
			st.state = AIState::ROAMING;
			break;
		}

		switch (st.state)
		{
		case AIState::ROAMING:
		{
			const float omega = ROAM_SINE_SPEED;
			const float amp = ROAM_AMPLITUDE;
			const float vy = amp * omega * std::cos(st.t * omega);
			const float absVy = std::fabs(vy);

			if (absVy < ROAM_EPS)
			{
				ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);
				break;
			}

			const float dirY = (vy > 0.f) ? 1.f : -1.f;
			const float speed = std::min(absVy, ROAM_SPEED_MAX);
			ApplyVelocity(eID, { 0.f, dirY }, speed, true);
			break;
		}

		case AIState::CHASING:
		{
			if (!hasPlayer)
			{
				ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);
				break;
			}

			Vec2 to = { playerPos.x - selfPos.x, playerPos.y - selfPos.y };
			if (DistSq(selfPos, playerPos) <= STOP_DISTANCE * STOP_DISTANCE)
			{
				ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);
			}
			else
			{
				ApplyVelocity(eID, to, CHASE_SPEED, true);
			}
			break;
		}

		case AIState::ATTACKING:
		{
			ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);

			if (!hasPlayer) break;

			if (!st.hitQueued)
			{
				st.windup = MELEE_WINDUP;
				st.hitQueued = true;
			}

			if (st.windup <= 0.f)
			{
				Vec2 to = { playerPos.x - selfPos.x, playerPos.y - selfPos.y };
				float len2 = RG_LengthSquared(to);
				if (len2 > 0.0001f)
				{
					if (st.attackMode == AttackMode::MELEE_ONLY)
					{
						ApplyMeleeDamage(eID, player);
						st.cooldown = MELEE_COOLDOWN;
						st.didAttack = true;
					}
					else
					{
						float inv = 1.f / RG_Sqrt(len2);
						Vec2 dir = { to.x * inv, to.y * inv };

						if (st.shootTimer <= 0.f && st.ammo > 0)
						{
							SpawnEnemyProjectile(eID, selfPos, dir, PROJECTILE_SPEED);
							st.ammo -= 1;
							st.shootTimer = SHOOT_COOLDOWN;
							st.cooldown = SHOOT_COOLDOWN;
							st.didAttack = true;
						}
					}
				}
			}

			break;
		}

		case AIState::COOLDOWN:
			ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);
			break;

		case AIState::RETURNING:
		{
			Vec2 toBase = { basePos.x - selfPos.x, basePos.y - selfPos.y };
			ApplyVelocity(eID, toBase, RETURN_SPEED, true);
			break;
		}

		default:
			ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);
			break;
		}
	}

	inline void End(Entity eID)
	{
		g_ai.erase(eID);
		if (Registry::Get().Has_Component<Movement::MovementComponent>(eID))
			Registry::Get().Get_Component<Movement::MovementComponent>(eID).ResetFrameIntent();
	}
}

void EnemySystem::Initialize()
{
	BehaviourFCT enemy;
	enemy.Init = [](Entity e) { EnemyAI::Start(e); };
	enemy.Update = [](Entity e, float dt) { EnemyAI::Update(e, dt); };
	enemy.End = [](Entity e) { EnemyAI::End(e); };
	LogicSystem::Get().RegisterScript("ManfredEnemy", enemy, ScriptID::GHOUL);
}

void EnemySystem::Update(float dt)
{
	(void)dt;
}

void EnemySystem::Shutdown()
{
}