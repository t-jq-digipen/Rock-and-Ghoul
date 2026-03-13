/* ========================================================
 * @file	    EnemySystem.cpp
 * @author	    Manfred Sim         | manfred.sim@digipen.edu
 * @co-author	Yasmin Tazi			| yasmin.t@digipen.edu
 * @date	    28 JAN 2026
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
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */


#include "reginegine.h"
#include "EnemySystem.h"
#include "Registry.h"
#include "Component.h"
#include "Remath.h"
#include "BehaviourFCT.h"
#include "SceneManager.h"
#include "Enemy.h"
#include <unordered_map>
#include "CollisionManager.h"
#include "PlayerSystem.h"
#include "CombatSystem.h"
#include "PhysicsSystem.h"
#include "ReplayManager.h"

#include "BehaviourComponent.h"

static constexpr float DETECT_RADIUS = 400.f;
static constexpr float LOSE_RADIUS = 360.f;
static constexpr float MAX_SHOOT_RANGE = 600.f;
static constexpr float STOP_DISTANCE = 8.f;

static constexpr float ROAM_SINE_SPEED = 2.f;
static constexpr float ROAM_SPEED_MAX = 200.f;
static constexpr float ROAM_AMPLITUDE = 100.f;
static constexpr float ROAM_EPS = 0.5f;

static constexpr float CHASE_SPEED = 300.f;
static constexpr float RETURN_SPEED = 600.f;

static constexpr float SPIRIT_MELEE_RANGE = 200.f;
static const Vec2	   SPIRIT_MELEE_HITBOX_SIZE = { 250.f, 250.f };
static constexpr float SPIRIT_MELEE_COOLDOWN = 0.8f;
static constexpr float SPIRIT_MELEE_WINDUP = 0.65f;
static constexpr float SPIRIT_MELEE_DURATION = 1.f;
static constexpr float SPIRIT_MELEE_DAMAGE = 10.f;
static constexpr float SPIRIT_MELEE_LIFETIME = 0.2f;
static constexpr float SPIRIT_MELEE_KNOCKBACK = 500.f;

AttackProperties SPIRIT_MELEE_PROPERTIES{
	INVALID_ENTITY,
	TagComponent{T_PLAYER},
	SPIRIT_MELEE_DAMAGE,
	2000.f,
	Element::NONE,
	SPIRIT_MELEE_LIFETIME,
	false
};

static constexpr float SPIRIT_RANGED_WINDUP = 0.75f;
static constexpr float SPIRIT_RANGED_DURATION = 1.f;
static constexpr float SPIRIT_PROJECTILE_DAMAGE = 10.f;
static constexpr float SPIRIT_PROJECTILE_LIFETIME = 10.f;
static constexpr float SPIRIT_PROJECTILE_SPAWN = 0.1f;
static constexpr float SPIRIT_PROJECTILE_EXPIRE = 0.5f;

AttackProperties SPIRIT_PROJECTILE_PROPERTIES{
	INVALID_ENTITY,
	TagComponent{T_PLAYER},
	SPIRIT_PROJECTILE_DAMAGE,
	100.f,
	Element::NONE,
	SPIRIT_PROJECTILE_LIFETIME,
	false
};

const ProjectileRuntime SPIRIT_PROJECTILE_RUNTIME(
	SPIRIT_PROJECTILE_SPAWN,
	SPIRIT_PROJECTILE_LIFETIME,
	SPIRIT_PROJECTILE_EXPIRE
);

static constexpr float SHOOT_RANGE = 240.f;
static constexpr float SHOOT_COOLDOWN = 1.2f;
static constexpr float PROJECTILE_SPEED = 600.f;

static constexpr float SPIRIT_DEATH_DURATION = 1.0f;

static constexpr int ANIM_DEATH_STATE = 2;
static constexpr int ANIM_STAGGER_STATE = 3;

enum SPIRIT_ANIM_STATE : unsigned int
{
	IDLE = 0,
	MELEE = 1,
	DEATH = 2,
	STAGGER = 3,
	RANGED = 4,
};

Entity targetPlayer = INVALID_ENTITY;

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


	// target
	Entity target{ INVALID_ENTITY }; // set the player!

	float deathTimer{ 0.f };
};


struct ElemBarrier
{
	Entity barrierEntity = INVALID_ENTITY;
	Element element = Element::NONE;
	bool active = true;
};

static std::unordered_map<Entity, EnemyRuntime> g_ai;
static std::unordered_map<Entity, ElemBarrier> g_elemBarriers{};
std::vector<Entity> toRemoveBarriers{};

Entity CreateBarrier(Entity eID, Element elem)
{
	std::string filepath;

	switch (elem)
	{
	case Element::FIRE:
		filepath = FilePath::PREFABS_FOLDER + "Barrier_Fire.json";
		break;

	case Element::ICE:
		filepath = FilePath::PREFABS_FOLDER + "Barrier_Ice.json";
		break;

	case Element::LIGHTNING:
		filepath = FilePath::PREFABS_FOLDER + "Barrier_Lightning.json";
		break;

	case Element::WIND:
		filepath = FilePath::PREFABS_FOLDER + "Barrier_Wind.json";
		break;

	default:
		std::cout << "Invalid element for barrier creation!\n";
		return INVALID_ENTITY;
	}

	const TransformComponent& transComp = Registry::Get().Get_Component<TransformComponent>(eID);
	Entity barrier = JsonLoad::LoadEntityFromFile(filepath, transComp.pos.x, transComp.pos.y-0.1f);

	if (barrier != INVALID_ENTITY) {
		g_elemBarriers[eID] = ElemBarrier{ barrier, elem, true };
	}
	return barrier;
}

bool HasBarrier(Entity eID)
{
	if (g_elemBarriers.find(eID) == g_elemBarriers.end())
		return false;

	if (!g_elemBarriers[eID].active)
		return false;

	return true;
}

bool BreakBarrier(Entity eID, Element elem)
{
	// should already exist, should only call this after checking if HasBarrier(Entity) is true

	if (elem == Element::NONE)
		return false;

	ElemBarrier& barrier = g_elemBarriers[eID];
	
	if (!barrier.active || barrier.element != elem)
		return false;

	// Queue remove barrier, let system destroy later
	barrier.active = false;
	Registry::Get().Remove_Component<ElementComponent>(eID);
	return true;
}

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
}

static inline void SpawnEnemyProjectile(Entity enemy, Vec2 pos, Vec2 dir, float speed)
{
	(void)enemy; (void)pos; (void)dir; (void)speed;
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
		if (ReplayManager::Get().IsPlaying()) return;
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

		// Check if enemy is stunned
		if (Registry::Get().Has_Component<StatusEffectComponent>(eID)) {
			auto& statusEffect = Registry::Get().Get_Component<StatusEffectComponent>(eID);
			if (statusEffect.isStunned) {
				// Clear movement direction so MovementSystem applies 0 velocity
				auto& moveComp = Registry::Get().Get_Component<Movement::MovementComponent>(eID);
				moveComp.SetMovementDirection({ 0.f, 0.f });
				// Skip AI so it doesn't override
				return;
			}
		}

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
		const float atkRange = (st.attackMode == AttackMode::MELEE_ONLY) ? SPIRIT_MELEE_RANGE : SHOOT_RANGE;
		const float atkSq = atkRange * atkRange;
		if (hasPlayer)
		{
			//std::cout << "[DBG] e=" << (int)eID
			//	<< " state=" << (int)st.state
			//	<< " d=" << std::sqrt(dPlayerSq)
			//	<< " atk=" << std::sqrt(atkSq)
			//	<< " mode=" << (st.attackMode == AttackMode::MELEE_ONLY ? "MELEE" : "RANGED")
			//	<< "\n";
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
				// Use MovementComponent speed (respects Ice slow effect)
				float currentSpeed = CHASE_SPEED;
				if (Registry::Get().Has_Component<Movement::MovementComponent>(eID)) {
					currentSpeed = Registry::Get().Get_Component<Movement::MovementComponent>(eID).GetMoveSpeed();
				}
				ApplyVelocity(eID, to, currentSpeed, true);
			}
			break;
		}

		case AIState::ATTACKING:
		{
			ApplyVelocity(eID, { 0.f,0.f }, 0.f, false);

			if (!hasPlayer) break;

			if (!st.hitQueued)
			{
				st.windup = SPIRIT_MELEE_WINDUP;
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
						st.cooldown = SPIRIT_MELEE_COOLDOWN;
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




void AssignTarget() {
	for (auto& [entity, comp] : Registry::Get().Get_ComponentMap<TagComponent>())
	{
		if (comp.tag == T_PLAYER && Registry::Get().Has_Component<TransformComponent>(entity))
		{
			targetPlayer = entity;
			return;
		}
	}

	targetPlayer = INVALID_ENTITY;
	std::cout << "[EnemySystem] No player found to assign as target!\n";
}

void ClearTarget() {
	targetPlayer = INVALID_ENTITY;
}

struct SpiritRuntime
{
	SpiritState state{ SpiritState::SPIRIT_IDLE };

	bool willAttack{ false };

	float stateTime{ 0.f };

	float cooldown{ 0.f };
};

std::unordered_map<Entity, SpiritRuntime> g_spirits;


namespace SpiritScript {
	// Register spirit runtime and assign target player
	inline void Start(Entity eID) {
		std::cout << "Spirit script started for entity " << eID << std::endl;
		CombatSystem::Get().RegisterCombatant(eID);
		g_spirits[eID] = SpiritRuntime();

		if (Registry::Get().Has_Component<ElementComponent>(eID)) {
			// auto& elemComp = Registry::Get().Get_Component<ElementComponent>(eID);
			// auto& transformComp = Registry::Get().Get_Component<TransformComponent>(eID);

			// Entity barrier = CreateBarrier(eID, elemComp.element);


		}
	}

	inline void Update(Entity eID, float dt) {
		if (ReplayManager::Get().IsPlaying()) return;
		if (targetPlayer == INVALID_ENTITY) {
			// No target player assigned, skipping update for spirit
			return;
		}
		// registry
		Registry& registry = Registry::Get();

		// Check if spirit is stunned
		if (registry.Has_Component<StatusEffectComponent>(eID)) {
			auto& statusEffect = registry.Get_Component<StatusEffectComponent>(eID);
			if (statusEffect.isStunned) {
				// Clear movement direction so MovementSystem applies 0 velocity
				if (registry.Has_Component<Movement::MovementComponent>(eID)) {
					auto& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);
					moveComp.SetMovementDirection({ 0.f, 0.f });
				}
				// Skip AI so it doesn't override
				return;
			}
		}

		if (!registry.Has_Component<Movement::MovementComponent>(eID) ||
			!registry.Has_Component<TransformComponent>(eID) ||
			!registry.Has_Component<HealthComponent>(eID) ||
			!registry.Has_Component<RenderComponent>(eID))
		{
			// missing required components
			return;
		}

		auto& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);
		// auto& transComp = registry.Get_Component<TransformComponent>(eID);
		auto& healthComp = registry.Get_Component<HealthComponent>(eID);

		RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(eID);
		Sprite::SpriteSheet& spriteSheet = renderComp.GetSpriteSheet();

		SpiritRuntime& spiritLogic = g_spirits[eID];

		// ari shifted these 2 things about bc the audio needs the selfPos declaration on top
		// get player pos and spirit pos
		Vec2 playerPos = GetPos(targetPlayer);
		Vec2 selfPos = GetPos(eID);

		if (healthComp.currhp <= 0.f) {
			// spirit state not dead yet
			if (spiritLogic.state != SpiritState::SPIRIT_DEAD) {
				spiritLogic.state = SpiritState::SPIRIT_DEAD;
				spiritLogic.stateTime = 0.f;
				spriteSheet.SetState(2);
				registry.Add_Component<LifetimeComponent>(eID, LifetimeComponent{ 0.f, SPIRIT_DEATH_DURATION });
				moveComp.ResetFrameIntent();
				AudioSystem::PlaySoundAt("enemydie", selfPos.x, selfPos.y, 0.3f); //die sound
			}
			else // spirit is dead 
			{
				spiritLogic.stateTime += dt;
				spriteSheet.SetState(2);
				//if (spiritLogic.stateTime >= SPIRIT_DEATH_DURATION) {
				//	// remove spirit entity
				//	//g_spirits.erase(eID);
				//}
			}
			// no need check the rest bc this fella die alr
			return;
		}

		Vec2 directionToPlayer = { playerPos.x - selfPos.x, playerPos.y - selfPos.y };


		// always face player
		bool faceRight = directionToPlayer.x >= 0.f;
		spriteSheet.Sprite().SetFlipX(faceRight);
		

		// get hit lol
		if (CombatSystem::Get().IsCombatantStaggered(eID)) {
			// stagger this fella
			spiritLogic.state = SpiritState::SPIRIT_HIT;
			spiritLogic.stateTime = 0.f;
			CombatSystem::Get().SetCombatantStagger(eID, false);
			AudioSystem::PlaySoundAt("enemyhit", selfPos.x, selfPos.y, 0.2f); //hitsound
		}

		if (spiritLogic.state == SpiritState::SPIRIT_IDLE) {

			spriteSheet.SetState(SPIRIT_ANIM_STATE::IDLE);
			float dPlayerSq = std::numeric_limits<float>::infinity();
			dPlayerSq = DistSq(selfPos, playerPos);
			float detectSq = DETECT_RADIUS * DETECT_RADIUS;
			if (dPlayerSq <= detectSq) 
			{
				spiritLogic.state = SpiritState::SPIRIT_CHASE;
			}
			else // bing chilling no detect player
			{
				moveComp.ResetFrameIntent();
				return;
			}



		}


		moveComp.ResetFrameIntent();
		switch (spiritLogic.state)
		{
		case SpiritState::SPIRIT_CHASE:
		{
			spriteSheet.SetState(SPIRIT_ANIM_STATE::IDLE);
			moveComp.ResetFrameIntent();
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveComp.SetIsMoving(true);
			moveComp.SetMovementDirection(moveDir);

			// if close enough, time to smack
			float dPlayerSq = DistSq(selfPos, playerPos);
			float meleeSq = SPIRIT_MELEE_RANGE * SPIRIT_MELEE_RANGE;
			if (dPlayerSq <= meleeSq) {
				spiritLogic.willAttack = true;
				spiritLogic.stateTime = 0.f;
				spiritLogic.state = SpiritState::SPIRIT_MELEE;
			}
		}
		break;
		// end case SPIRIT_CHASE

		case SpiritState::SPIRIT_MELEE:
		{
			spriteSheet.SetState(SPIRIT_ANIM_STATE::MELEE);
			spiritLogic.stateTime += dt;

			if (spiritLogic.stateTime >= SPIRIT_MELEE_WINDUP && spiritLogic.willAttack) {
				spiritLogic.willAttack = false;
				AudioSystem::PlaySoundAt("enemyattack", selfPos.x, selfPos.y, 0.3f);

				Vec2 offset = { 0.f, 0.f };
				if (faceRight) {
					offset.x += SPIRIT_MELEE_RANGE / 2.f;
				}
				else {
					offset.x -= SPIRIT_MELEE_RANGE / 2.f;
				}

				AttackProperties props = SPIRIT_MELEE_PROPERTIES;
				props.owner = eID;
				CombatSystem::Get().SpawnHitBox(
					props,
					offset,
					SPIRIT_MELEE_HITBOX_SIZE
				);
			}

			if (spiritLogic.stateTime >= SPIRIT_MELEE_DURATION) {
				spiritLogic.stateTime = 0.f;
				spiritLogic.state = SpiritState::SPIRIT_ESCAPE;
				moveComp.ResetFrameIntent();
			}

		}
		break;
		// end case SPIRIT_MELEE

		case SpiritState::SPIRIT_ESCAPE:
		{
			spiritLogic.stateTime += dt;
			spriteSheet.SetState(SPIRIT_ANIM_STATE::IDLE);
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveDir.x = -moveDir.x;
			moveDir.y = -moveDir.y;
			moveComp.SetIsMoving(true);
			if (DistSq(selfPos, playerPos) > MAX_SHOOT_RANGE * MAX_SHOOT_RANGE) moveDir.x = -moveDir.x;
			moveComp.SetMovementDirection(moveDir);

			if (spiritLogic.stateTime >= 1.5f) {
				// add chance for range later
				spiritLogic.willAttack = true;

				spiritLogic.stateTime = 0.f;
				spiritLogic.state = healthComp.maxhp > 50.f ? SpiritState::SPIRIT_CHASE : SpiritState::SPIRIT_RANGE;
				moveComp.ResetFrameIntent();
			}
		}
		break;
		// end case SPIRIT_ESCAPE

		case SpiritState::SPIRIT_RANGE:
		{
			spriteSheet.SetState(SPIRIT_ANIM_STATE::RANGED);
			spiritLogic.stateTime += dt;


			if (spiritLogic.stateTime >= SPIRIT_RANGED_WINDUP && spiritLogic.willAttack) {
				spiritLogic.willAttack = false;

				//same as soundwav
				// Create visual entity (for rendering/physics)
				Entity spit = JsonLoad::LoadEntityFromFile(
					FilePath::PREFABS_FOLDER + "SpiritSpit.json",
					selfPos.x, selfPos.y
				);


				// if dont have smth wrong with the prefab... but just to prevent crash
				if (registry.Has_Component<Physics::PhysicsComponent>(spit))
				{
					TransformComponent& spitTrans = registry.Get_Component<TransformComponent>(spit);
					Physics::PhysicsComponent& spitPhy = registry.Get_Component<Physics::PhysicsComponent>(spit);
					// make projectile
					Vec2 dir = playerPos - selfPos;
					dir = RG_Normalize(dir);
					float angleRad = std::atan2(dir.y, dir.x);
					// angleDeg = angleRad * (180.0f / RG_PI);
					spitTrans.rot = angleRad;

					dir = RG_Scale(dir, PROJECTILE_SPEED);
					spitPhy.Set_Vel(dir);
				}

				AttackProperties props = SPIRIT_PROJECTILE_PROPERTIES;
				props.owner = spit;
				CombatSystem::Get().RegisterProjectile(spit, props, SPIRIT_PROJECTILE_RUNTIME);
			}

			if (spiritLogic.stateTime >= SPIRIT_MELEE_DURATION) {
				spiritLogic.stateTime = 0.f;
				spiritLogic.state = SpiritState::SPIRIT_ESCAPE;
				moveComp.ResetFrameIntent();
			}
		}
		break;
		// end case SPIRIT_RANGE

		case SpiritState::SPIRIT_HIT:
		{
			spriteSheet.SetState(SPIRIT_ANIM_STATE::STAGGER);
			spiritLogic.stateTime += dt;

			spiritLogic.willAttack = false;
			if (spiritLogic.stateTime >= 0.5f) {
				spiritLogic.stateTime = 1.f;
				spiritLogic.state = SpiritState::SPIRIT_ESCAPE;
				moveComp.ResetFrameIntent();
			}

		}
		break;
		// end case SPIRIT_HIT

		default:
			break;

		} // end switch spirit state

	}

	inline void End(Entity eID) {
		std::cout << "Spirit script ended for entity " << eID << std::endl;
		g_spirits.erase(eID);
		if (HasBarrier(eID)) {
			g_elemBarriers[eID].active = false;
		}
		CombatSystem::Get().RemoveCombatant(eID);
	}
}

// =======================================================
// FOR ENVIRONMENTAL OBSTACLES (e.g. speakers)
#pragma region Obstacles


//enum OBSTACLE_ANIM_STATE : unsigned int
//{
//	OBS_IDLE = 0,
//	OBS_BURN,
//	OBS_EXPLODE
//};

static constexpr float OBSTACLE_DEATH_LIFETIME = 0.05f;
static constexpr float OBSTACLE_EXPLOSION_DURATION = .75f;

struct ObstacleRuntime
{
	ObstacleState state{ ObstacleState::OBSTACLE_IDLE };
	float stateTime{ 0.f };
};

static std::unordered_map<Entity, ObstacleRuntime> g_obstacles;
static std::unordered_map<Entity, ObstacleState> prevObstacleState;
static std::unordered_map<Entity, unsigned> prevAnimState;

// =======================================================

// Ensure that obstacle stays still
static inline void MakeObstacleStationary(Entity eID)
{
	Registry& registry = Registry::Get();
	if (!registry.Has_Component<Movement::MovementComponent>(eID))
		return;

	auto& mv = registry.Get_Component<Movement::MovementComponent>(eID);
	mv.ResetFrameIntent();
	mv.SetIsMoving(false);
	mv.SetMovementDirection({ 0.f, 0.f });
	mv.SetMoveSpeed(0.f);
}

// Set obstacle spritesheet
static inline void SetObstacleAnim(Entity eID, int row, int col = 0)
{
	Registry& registry = Registry::Get();
	if (!registry.Has_Component<RenderComponent>(eID))
	{
		return;
	}

	auto& rc = registry.Get_Component<RenderComponent>(eID);
	auto& sheet = rc.GetSpriteSheet();
	sheet.Sprite().Update_ImageSize();
	
	int internalRow = row;
	if (internalRow < 0) internalRow = 0;
	if (internalRow >= sheet.GetTotalRows()) internalRow = sheet.GetTotalRows() - 1;
	if (col < 0) col = 0;
	if (col >= sheet.GetTotalCols()) col = sheet.GetTotalCols() - 1;

	const unsigned int currState = static_cast<unsigned int>(sheet.GetCurrentState());
	sheet.EditFrames(internalRow, col, col, 0.f, false);
	sheet.SetState(currState);

	const auto& img = sheet.Sprite().Get_Image();
	sheet.UpdateUV(img.widthPixels, img.heightPixels);
}

// Set obstacle death lifetime once before deletion
static inline void DeactivateObstacle(Entity eID)
{
	Registry& registry = Registry::Get();
	if (registry.Has_Component<LifetimeComponent>(eID))
	{
		registry.Get_Component<LifetimeComponent>(eID) = LifetimeComponent{ 0.f, OBSTACLE_DEATH_LIFETIME };
		return;
	}

	registry.Add_Component<LifetimeComponent>(eID, LifetimeComponent{ 0.f, OBSTACLE_DEATH_LIFETIME });
}

namespace ObstacleScript
{
	std::unordered_map<Entity, Entity> obstacleStatusEffects{};

	inline void Start(Entity eID)
	{
		g_obstacles[eID] = ObstacleRuntime{};
		MakeObstacleStationary(eID);
		obstacleStatusEffects[eID] = JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "ObstacleStatusFx.json", 0.f, 0.f);
		prevObstacleState[eID] = g_obstacles[eID].state;
		prevAnimState[eID] = (unsigned)-1;
	}

	inline void Update(Entity eID, float dt)
	{
		if (ReplayManager::Get().IsPlaying()) return;

		Registry& registry = Registry::Get();

		if (!registry.Has_Component<HealthComponent>(eID))
			return;

		unsigned animState = 0;

		TransformComponent& transComp = registry.Get_Component<TransformComponent>(eID);
		TransformComponent& statusTrans = registry.Get_Component<TransformComponent>(obstacleStatusEffects[eID]);
		statusTrans.pos = transComp.pos;

		auto& rt = g_obstacles[eID];
		const bool canOverrideAnimState =
			(rt.state != ObstacleState::OBSTACLE_EXPLODE) &&
			(rt.state != ObstacleState::OBSTACLE_DESTROYED);
		const bool enteredState = (prevObstacleState[eID] != rt.state);
		prevObstacleState[eID] = rt.state;

		auto& hp = registry.Get_Component<HealthComponent>(eID);
		auto& sheet = registry.Get_Component<RenderComponent>(eID).GetSpriteSheet();

		// MakeObstacleStationary(eID);

		// Elemental status effects active checks

		bool isStunned = false;
		bool isBurning = false;
		bool isFrozen = false;
		bool isPushed = false;
		if (registry.Has_Component<StatusEffectComponent>(eID))
		{
			StatusEffectComponent& statusEffect = registry.Get_Component<StatusEffectComponent>(eID);
			isStunned = statusEffect.activeEffect == Element::LIGHTNING;
			isBurning = statusEffect.activeEffect == Element::FIRE;
			isFrozen = statusEffect.activeEffect == Element::ICE;
			isPushed = statusEffect.activeEffect == Element::WIND;

			switch (statusEffect.activeEffect)
			{
			case Element::FIRE:
				registry.Get_Component<RenderComponent>(obstacleStatusEffects[eID]).GetSpriteSheet().SetState(1);
				break;
			case Element::ICE:
				registry.Get_Component<RenderComponent>(obstacleStatusEffects[eID]).GetSpriteSheet().SetState(2);
				break;
			case Element::LIGHTNING:
				registry.Get_Component<RenderComponent>(obstacleStatusEffects[eID]).GetSpriteSheet().SetState(3);
				break;
			case Element::WIND:
			default:
				registry.Get_Component<RenderComponent>(obstacleStatusEffects[eID]).GetSpriteSheet().SetState(0);
				break;
			}
		}
		else
		{
			registry.Get_Component<RenderComponent>(obstacleStatusEffects[eID]).GetSpriteSheet().SetState(0);
		}

		if (canOverrideAnimState)
		{
			if (isBurning)					animState = 2;	// Burning
			else if (hp.currhp < hp.maxhp)	animState = 1;	// Damaged
			else							animState = 0;	// Idle
		}

		switch (rt.state)
		{
		case ObstacleState::OBSTACLE_IDLE:
		{
			//SetObstacleAnim(eID, OBSTACLE_ANIM_STATE::OBS_IDLE);

			if (isBurning)
			{
				rt.state = ObstacleState::OBSTACLE_BURNING;
				rt.stateTime = 0.f;
				break;
			}
			else if (isPushed)
			{
				rt.state = ObstacleState::OBSTACLE_PUSHED;
				rt.stateTime = 0.f;
				break;
			}

			if (hp.currhp <= 0.f)
			{
				hp.currhp = 0.f;
				hp.willHit = false;
				hp.isVulnerable = false;
				rt.state = ObstacleState::OBSTACLE_DESTROYED;
				rt.stateTime = 0.f;
				DeactivateObstacle(eID);
			}
			break;
		}

		case ObstacleState::OBSTACLE_BURNING:
		{
			//SetObstacleAnim(eID, OBSTACLE_ANIM_STATE::OBS_BURN);

			if (!isBurning && hp.currhp > 0.f)
			{
				rt.state = ObstacleState::OBSTACLE_IDLE;
				rt.stateTime = 0.f;
				break;
			}

			if (hp.currhp <= 0.f)
			{
				hp.currhp = 0.f;
				hp.willHit = false;
				hp.isVulnerable = false;
				rt.state = ObstacleState::OBSTACLE_EXPLODE;
				rt.stateTime = 0.f;
			}
			break;
		}

		case ObstacleState::OBSTACLE_EXPLODE:
		{
			//SetObstacleAnim(eID, OBSTACLE_ANIM_STATE::OBS_EXPLODE);
			rt.stateTime += std::max(0.f, dt);
			//sheet.Sprite().Update_ImageSize();
			//const auto& img = sheet.Sprite().Get_Image();
			animState = 3;
			/*if (enteredState)
			{
				sheet.SetState(animState);
				sheet.EditFrames(1, 0, 7, 0.2f, false);
				sheet.ResetTimer();
			}*/
			/*sheet.UpdateFrame(dt);
			sheet.UpdateUV(img.widthPixels, img.heightPixels);*/
			if (rt.stateTime >= OBSTACLE_EXPLOSION_DURATION)
			{
				rt.state = ObstacleState::OBSTACLE_DESTROYED;
				rt.stateTime = 0.f;
				DeactivateObstacle(eID);
			}
			break;
		}

		case ObstacleState::OBSTACLE_PUSHED:
		{
			if (!isPushed)
			{
				rt.state = ObstacleState::OBSTACLE_IDLE;
			}
			break;
		}

		case ObstacleState::OBSTACLE_DESTROYED:
		{
			break;
		}
		default:
		{
			// No-op: obstacle is already scheduled for removal / deactivated.
			break;
		}
		}

		unsigned& prev = prevAnimState[eID];
		/*if (prev != animState)
		{
			registry.Get_Component<RenderComponent>(eID).GetSpriteSheet().SetState(animState);
			prev = animState;
		}*/
		registry.Get_Component<RenderComponent>(eID).GetSpriteSheet().SetState(animState);
	}

	inline void End(Entity eID)
	{
		g_obstacles.erase(eID);
		// expire status effect display
		Registry::Get().Remove_Entity(obstacleStatusEffects[eID]);
		obstacleStatusEffects.erase(eID);
		prevObstacleState.erase(eID);
		prevAnimState.erase(eID);
	}
}

#pragma endregion Obstacles


void UpdateBarriers()
{
	for (auto& [owner, barrier] : g_elemBarriers)
	{
		// If owner dead or barrier inactive, remove
		if (!Registry::Get().Has_Component<TransformComponent>(owner) || !barrier.active || !Registry::Get().Has_Component<TransformComponent>(barrier.barrierEntity))
		{
			toRemoveBarriers.push_back(owner);
			continue;
		}

		// update position
		Vec2 ownerPos = GetPos(owner);
		Registry::Get().Get_Component<TransformComponent>(barrier.barrierEntity).pos = { ownerPos.x, ownerPos.y - 0.1f };
	}

	if (!toRemoveBarriers.empty())
	{
		for (Entity e : toRemoveBarriers)
		{
			Registry::Get().Remove_Entity(g_elemBarriers[e].barrierEntity);
			g_elemBarriers.erase(e);
		}
		toRemoveBarriers.clear();
	}
}

// should only have one ghoul so... not using a map just single instance
namespace GhoulScript {
	// const stuff!
	constexpr float GHOUL_BARRIER_COOLDOWN = 5.f;
	constexpr float GHOUL_MELEE_RANGE = 200.f;
		
	constexpr float GHOUL_BARRIER_CASTTIME = 1.f;

	constexpr float GHOUL_MELEE_DURATION = 1.f;
	constexpr float GHOUL_MELEE_WINDUP = 0.5f;
	const Vec2	    GHOUL_MELEE_HITBOX_SIZE = { 250.f, 250.f };

	constexpr float GHOUL_RANGE_DURATION = 1.f;
	constexpr float GHOUL_RANGE_CASTTIME = 0.5f;

	constexpr float GHOUL_STAGGER_DURATION = 1.f;

	constexpr float GHOUL_PHASE1_COOLDOWN = 2.f;
	constexpr float GHOUL_PHASE2_COOLDOWN = 1.f;
	constexpr float GHOUL_PHASE3_COOLDOWN = 0.5f;

	bool inCombat = false;
	bool hasStartingDialogueBeenTriggered = false;
	bool hasDeathDialogueBeenTriggered = false;
	bool waitingForDialogue = false;
	static bool IsAnyDialogueVisibleInGroup(GuiSystem& gui, int groupID)
	{
		for (auto& g : gui.GetGroups())
		{
			if (g.groupID != groupID) continue;
			for (int memberID : g.memberIDs)
			{
				for (auto& d : gui.Dialogues())
				{
					if (d.ID == memberID && d.isVisible) return true;
				}
			}
			return false;
		}
		return false;
	}

	AttackProperties GHOUL_MELEE_PROPERTIES = {
		INVALID_ENTITY, // set at runtime
		T_PLAYER,
		15.f,
		500.f,
		Element::NONE,
		0.2f,
		false
	};

	struct GhoulRuntime
	{
		GhoulState state{ GhoulState::GHOUL_SPAWN };
		GhoulAction nextAction{ GhoulAction::GHOUL_ACTION_RANGE };
		float stateTime{ 0.f };

		// cooldowns
		float attackCooldown = 0.f;
		float barrierCooldown = 0.f;

		// barrier
		Element barrierElem = Element::NONE;
		bool hasBarrier = false;

		// stagger count (hit 3x to stagger)
		int staggerCount = 0;
		
		// phases
		int phase = 1;

		bool willAttack = false;


	};

	Entity ghoulEntity = INVALID_ENTITY;
	GhoulRuntime ghoulRuntime;

	// to get a new elem that isnt previous
	Element GetBarrierElement(Element previous) {
		Element result;
		do {
			result = static_cast<Element>((rand() % 4) + 1);
		} while (result == previous);
		ghoulRuntime.barrierElem = result;
		return result;
	}

	void UpdateTimers(float dt, GhoulRuntime& rt)
	{
		rt.stateTime += dt;
		rt.attackCooldown = std::max(0.f, rt.attackCooldown - dt);
		rt.barrierCooldown = std::max(0.f, rt.barrierCooldown - dt);
	}

	void MovementLogic(Entity eID, float dt)
	{
		(void)eID; (void)dt;
	}

	void Phase1Logic(Entity eID, Movement::MovementComponent& moveComp, const Vec2& selfPos, const Vec2& directionToPlayer, unsigned int& animState, const float& dPlayerSq, bool& faceRight)
	{
		Registry& registry = Registry::Get();
		switch (ghoulRuntime.state)
		{
		case GHOUL_IDLE:
		{
			moveComp.ResetFrameIntent();
			animState = 0;
			if (ghoulRuntime.attackCooldown <= 0.f && ghoulRuntime.stateTime >= 0.5f && inCombat)
			{
				// between 0 to 1 random val
				float randVal = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				if (randVal < 0.5f) {
					// melee attack
					ghoulRuntime.willAttack = true;
					ghoulRuntime.nextAction = GhoulAction::GHOUL_ACTION_MELEE;
					moveComp.SetMoveSpeed(600.f);
				}
				else {
					// range attack
					ghoulRuntime.willAttack = true;
					ghoulRuntime.nextAction = GhoulAction::GHOUL_ACTION_RANGE;
					moveComp.SetMoveSpeed(400.f);
				}
				ghoulRuntime.state = GhoulState::GHOUL_MOVING;
				ghoulRuntime.stateTime = 0.f;
			}
		}
		break; // end case GHOUL_IDLE

		case GHOUL_MOVING:
		{
			moveComp.ResetFrameIntent();
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveComp.SetIsMoving(true);


			switch (ghoulRuntime.nextAction)
			{
			case GhoulAction::GHOUL_ACTION_MELEE:
			{
				moveComp.SetMovementDirection(moveDir);
				float meleeSq = GHOUL_MELEE_RANGE * GHOUL_MELEE_RANGE;
				if (dPlayerSq <= meleeSq) // smack
				{
					ghoulRuntime.willAttack = true;
					ghoulRuntime.state = GhoulState::GHOUL_MELEE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case GHOUL_ACTION_MELEE


			case GhoulAction::GHOUL_ACTION_RANGE:
			{

				moveComp.SetMovementDirection(moveDir);
				float rangeSq = MAX_SHOOT_RANGE * MAX_SHOOT_RANGE;
				if (dPlayerSq <= rangeSq) // shoot
				{
					moveDir.x = -moveDir.x;
				}
				// run for abit then shoots
				if (ghoulRuntime.stateTime >= .5f)
				{
					ghoulRuntime.willAttack = true;
					ghoulRuntime.state = GhoulState::GHOUL_RANGE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case GHOUL_ACTION_RANGE

			default:
				break;
			} // end switch next action

			animState = 1;
		}
		break; // end case GHOUL_MOVING

		case GHOUL_MELEE:
		{
			moveComp.ResetFrameIntent();
			animState = 2;
			if (ghoulRuntime.willAttack && ghoulRuntime.stateTime >= GHOUL_MELEE_WINDUP)
			{
				ghoulRuntime.willAttack = false;

				Vec2 offset = { 0.f, 0.f };
				if (faceRight) {
					offset.x += GHOUL_MELEE_RANGE / 2.f;
				}
				else {
					offset.x -= GHOUL_MELEE_RANGE / 2.f;
				}
				AttackProperties props = GHOUL_MELEE_PROPERTIES;
				props.owner = eID;
				CombatSystem::Get().SpawnHitBox(
					props,
					offset,
					GHOUL_MELEE_HITBOX_SIZE
				);
			}

			if (ghoulRuntime.stateTime >= GHOUL_MELEE_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				ghoulRuntime.attackCooldown = GHOUL_PHASE1_COOLDOWN;
			}


		}
		break; // end case GHOUL_MELEE

		case GHOUL_RANGE:
		{
			moveComp.ResetFrameIntent();
			animState = 3;
			if (ghoulRuntime.willAttack && ghoulRuntime.stateTime >= GHOUL_RANGE_CASTTIME)
			{
				ghoulRuntime.willAttack = false;


				//recycle spirit spit first
				// Create visual entity (for rendering/physics)
				Entity spit = JsonLoad::LoadEntityFromFile(
					FilePath::PREFABS_FOLDER + "SpiritSpit.json",
					selfPos.x, selfPos.y
				);

				// if dont have smth wrong with the prefab... but just to prevent crash
				if (registry.Has_Component<Physics::PhysicsComponent>(spit))
				{
					TransformComponent& spitTrans = registry.Get_Component<TransformComponent>(spit);
					Physics::PhysicsComponent& spitPhy = registry.Get_Component<Physics::PhysicsComponent>(spit);
					// make 1 projectile

					Vec2 dir = directionToPlayer;
					dir = RG_Normalize(dir);
					dir = RG_Scale(dir, PROJECTILE_SPEED);
					spitPhy.Set_Vel(dir);

					float angleRad = std::atan2(dir.y, dir.x);
					// angleDeg = angleRad * (180.0f / RG_PI);
					spitTrans.rot = angleRad;


					AttackProperties props = SPIRIT_PROJECTILE_PROPERTIES;
					props.owner = spit;
					CombatSystem::Get().RegisterProjectile(spit, props, SPIRIT_PROJECTILE_RUNTIME);

				}
				
			}

			if (ghoulRuntime.stateTime >= GHOUL_RANGE_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				ghoulRuntime.attackCooldown = GHOUL_PHASE1_COOLDOWN;
			}
		}
		break; // end case GHOUL_RANGE

		default:
			// should be handled already:
			// - Death
			// - Staggered
			// - Barrier
			//
			break;

		} // end switch ghoul state
	}

	void Phase2Logic(Entity eID, Movement::MovementComponent& moveComp, const Vec2& selfPos, const Vec2& directionToPlayer, unsigned int& animState, const float& dPlayerSq, bool& faceRight)
	{
		Registry& registry = Registry::Get();
		switch (ghoulRuntime.state)
		{
		case GHOUL_IDLE:
		{
			moveComp.ResetFrameIntent();
			animState = 0;
			if (ghoulRuntime.attackCooldown <= 0.f && ghoulRuntime.stateTime >= 0.5f && inCombat)
			{
				// between 0 to 1 random val
				float randVal = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				if (randVal < 0.5f) {
					// melee attack
					ghoulRuntime.willAttack = true;
					ghoulRuntime.nextAction = GhoulAction::GHOUL_ACTION_MELEE;
					moveComp.SetMoveSpeed(700.f);
				}
				else {
					// range attack
					ghoulRuntime.willAttack = true;
					ghoulRuntime.nextAction = GhoulAction::GHOUL_ACTION_RANGE;
					moveComp.SetMoveSpeed(400.f);
				}
				ghoulRuntime.state = GhoulState::GHOUL_MOVING;
				ghoulRuntime.stateTime = 0.f;
			}
		}
		break; // end case GHOUL_IDLE

		case GHOUL_MOVING:
		{
			moveComp.ResetFrameIntent();
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveComp.SetIsMoving(true);


			switch (ghoulRuntime.nextAction)
			{
			case GhoulAction::GHOUL_ACTION_MELEE:
			{
				moveComp.SetMovementDirection(moveDir);
				float meleeSq = GHOUL_MELEE_RANGE * GHOUL_MELEE_RANGE;
				if (dPlayerSq <= meleeSq) // smack
				{
					ghoulRuntime.willAttack = true;
					ghoulRuntime.state = GhoulState::GHOUL_MELEE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case GHOUL_ACTION_MELEE


			case GhoulAction::GHOUL_ACTION_RANGE:
			{

				moveComp.SetMovementDirection(moveDir);
				float rangeSq = MAX_SHOOT_RANGE * MAX_SHOOT_RANGE;
				if (dPlayerSq <= rangeSq) // shoot
				{
					moveDir.x = -moveDir.x;
				}
				// run for abit then shoots
				if (ghoulRuntime.stateTime >= .5f)
				{
					ghoulRuntime.willAttack = true;
					ghoulRuntime.state = GhoulState::GHOUL_RANGE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case GHOUL_ACTION_RANGE

			default:
				break;
			} // end switch next action

			animState = 1;
		}
		break; // end case GHOUL_MOVING

		case GHOUL_MELEE:
		{
			moveComp.ResetFrameIntent();
			animState = 2;
			if (ghoulRuntime.willAttack && ghoulRuntime.stateTime >= GHOUL_MELEE_WINDUP)
			{
				ghoulRuntime.willAttack = false;

				Vec2 offset = { 0.f, 0.f };
				if (faceRight) {
					offset.x += GHOUL_MELEE_RANGE / 2.f;
				}
				else {
					offset.x -= GHOUL_MELEE_RANGE / 2.f;
				}
				AttackProperties props = GHOUL_MELEE_PROPERTIES;
				props.owner = eID;
				CombatSystem::Get().SpawnHitBox(
					props,
					offset,
					GHOUL_MELEE_HITBOX_SIZE
				);
			}

			if (ghoulRuntime.stateTime >= GHOUL_MELEE_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				ghoulRuntime.attackCooldown = GHOUL_PHASE2_COOLDOWN;
			}


		}
		break; // end case GHOUL_MELEE

		case GHOUL_RANGE:
		{
			moveComp.ResetFrameIntent();
			animState = 3;
			if (ghoulRuntime.willAttack && ghoulRuntime.stateTime >= GHOUL_RANGE_CASTTIME)
			{
				ghoulRuntime.willAttack = false;

				// shotgun bangg
				for (int i = 0; i < 5; i++) {

					//recycle spirit spit first
					// Create visual entity (for rendering/physics)
					Entity spit = JsonLoad::LoadEntityFromFile(
						FilePath::PREFABS_FOLDER + "SpiritSpit.json",
						selfPos.x, selfPos.y
					);

					// if dont have smth wrong with the prefab... but just to prevent crash
					if (registry.Has_Component<Physics::PhysicsComponent>(spit))
					{
						TransformComponent& spitTrans = registry.Get_Component<TransformComponent>(spit);
						Physics::PhysicsComponent& spitPhy = registry.Get_Component<Physics::PhysicsComponent>(spit);
						// make projectile

						float angleOffset = (i - 2) * 0.1f; // -0.2, -0.1, 0, 0.1, 0.2 radians offset
						Vec2 dir = { directionToPlayer.x * cos(angleOffset) - directionToPlayer.y * sin(angleOffset),
									 directionToPlayer.x * sin(angleOffset) + directionToPlayer.y * cos(angleOffset) };
						dir = RG_Normalize(dir);
						dir = RG_Scale(dir, PROJECTILE_SPEED);
						spitPhy.Set_Vel(dir);

						float angleRad = std::atan2(dir.y, dir.x);
						// angleDeg = angleRad * (180.0f / RG_PI);
						spitTrans.rot = angleRad;


						AttackProperties props = SPIRIT_PROJECTILE_PROPERTIES;
						props.owner = spit;
						CombatSystem::Get().RegisterProjectile(spit, props, SPIRIT_PROJECTILE_RUNTIME);
					}
				}
			}

			if (ghoulRuntime.stateTime >= GHOUL_RANGE_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				ghoulRuntime.attackCooldown = GHOUL_PHASE2_COOLDOWN;
			}
		}
		break; // end case GHOUL_RANGE

		default:
			// should be handled already:
			// - Death
			// - Staggered
			// - Barrier
			//
			break;

		} // end switch ghoul state
	}

	void Phase3Logic(Entity eID, Movement::MovementComponent& moveComp, const Vec2& selfPos, const Vec2& directionToPlayer, unsigned int& animState, const float& dPlayerSq, bool& faceRight)
	{
		Registry& registry = Registry::Get();
		switch (ghoulRuntime.state)
		{
		case GHOUL_IDLE:
		{
			moveComp.ResetFrameIntent();
			animState = 0;
			if (ghoulRuntime.attackCooldown <= 0.f && ghoulRuntime.stateTime >= 0.5f && inCombat)
			{
				// between 0 to 1 random val
				float randVal = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				if (randVal < 0.5f) {
					// melee attack
					ghoulRuntime.willAttack = true;
					ghoulRuntime.nextAction = GhoulAction::GHOUL_ACTION_MELEE;
					moveComp.SetMoveSpeed(800.f);
				}
				else {
					// range attack
					ghoulRuntime.willAttack = true;
					ghoulRuntime.nextAction = GhoulAction::GHOUL_ACTION_RANGE;
					moveComp.SetMoveSpeed(400.f);
				}
				ghoulRuntime.state = GhoulState::GHOUL_MOVING;
				ghoulRuntime.stateTime = 0.f;
			}
		}
		break; // end case GHOUL_IDLE

		case GHOUL_MOVING:
		{
			moveComp.ResetFrameIntent();
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveComp.SetIsMoving(true);


			switch (ghoulRuntime.nextAction)
			{
			case GhoulAction::GHOUL_ACTION_MELEE:
			{
				moveComp.SetMovementDirection(moveDir);
				float meleeSq = GHOUL_MELEE_RANGE * GHOUL_MELEE_RANGE;
				if (dPlayerSq <= meleeSq) // smack
				{
					ghoulRuntime.willAttack = true;
					ghoulRuntime.state = GhoulState::GHOUL_MELEE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case GHOUL_ACTION_MELEE


			case GhoulAction::GHOUL_ACTION_RANGE:
			{

				moveComp.SetMovementDirection(moveDir);
				float rangeSq = MAX_SHOOT_RANGE * MAX_SHOOT_RANGE;
				if (dPlayerSq <= rangeSq) // shoot
				{
					moveDir.x = -moveDir.x;
				}
				// run for abit then shoots
				if (ghoulRuntime.stateTime >= .5f)
				{
					ghoulRuntime.willAttack = true;
					ghoulRuntime.state = GhoulState::GHOUL_RANGE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case GHOUL_ACTION_RANGE

			default:
				break;
			} // end switch next action

			animState = 1;
		}
		break; // end case GHOUL_MOVING

		case GHOUL_MELEE:
		{
			moveComp.ResetFrameIntent();
			animState = 2;
			if (ghoulRuntime.willAttack && ghoulRuntime.stateTime >= GHOUL_MELEE_WINDUP)
			{
				ghoulRuntime.willAttack = false;

				Vec2 offset = { 0.f, 0.f };
				if (faceRight) {
					offset.x += GHOUL_MELEE_RANGE / 2.f;
				}
				else {
					offset.x -= GHOUL_MELEE_RANGE / 2.f;
				}
				AttackProperties props = GHOUL_MELEE_PROPERTIES;
				props.owner = eID;
				CombatSystem::Get().SpawnHitBox(
					props,
					offset,
					GHOUL_MELEE_HITBOX_SIZE
				);
			}

			if (ghoulRuntime.stateTime >= GHOUL_MELEE_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				ghoulRuntime.attackCooldown = GHOUL_PHASE3_COOLDOWN;
			}


		}
		break; // end case GHOUL_MELEE

		case GHOUL_RANGE:
		{
			moveComp.ResetFrameIntent();
			animState = 3;
			if (ghoulRuntime.willAttack && ghoulRuntime.stateTime >= GHOUL_RANGE_CASTTIME)
			{
				ghoulRuntime.willAttack = false;

				// shotgun bangg
				for (int i = 0; i < 5; i++) {

					//recycle spirit spit first
					// Create visual entity (for rendering/physics)
					Entity spit = JsonLoad::LoadEntityFromFile(
						FilePath::PREFABS_FOLDER + "SpiritSpit.json",
						selfPos.x, selfPos.y
					);

					// if dont have smth wrong with the prefab... but just to prevent crash
					if (registry.Has_Component<Physics::PhysicsComponent>(spit))
					{
						TransformComponent& spitTrans = registry.Get_Component<TransformComponent>(spit);
						Physics::PhysicsComponent& spitPhy = registry.Get_Component<Physics::PhysicsComponent>(spit);
						// make projectile

						float angleOffset = (i - 2) * 0.1f; // -0.2, -0.1, 0, 0.1, 0.2 radians offset
						Vec2 dir = { directionToPlayer.x * cos(angleOffset) - directionToPlayer.y * sin(angleOffset),
									 directionToPlayer.x * sin(angleOffset) + directionToPlayer.y * cos(angleOffset) };
						dir = RG_Normalize(dir);
						dir = RG_Scale(dir, PROJECTILE_SPEED);
						spitPhy.Set_Vel(dir);

						float angleRad = std::atan2(dir.y, dir.x);
						// angleDeg = angleRad * (180.0f / RG_PI);
						spitTrans.rot = angleRad;


						AttackProperties props = SPIRIT_PROJECTILE_PROPERTIES;
						props.owner = spit;
						CombatSystem::Get().RegisterProjectile(spit, props, SPIRIT_PROJECTILE_RUNTIME);
					}
				}
			}

			if (ghoulRuntime.stateTime >= GHOUL_RANGE_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				ghoulRuntime.attackCooldown = GHOUL_PHASE3_COOLDOWN;
			}
		}
		break; // end case GHOUL_RANGE

		default:
			// should be handled already:
			// - Death
			// - Staggered
			// - Barrier
			//
			break;

		} // end switch ghoul state
	}

	void Start(Entity eID) {
		std::cout << "Spirit script started for entity " << eID << std::endl;
		CombatSystem::Get().RegisterCombatant(eID);

		ghoulRuntime = GhoulRuntime();
		ghoulEntity = eID;

		inCombat = false;
		hasStartingDialogueBeenTriggered = false;
		hasDeathDialogueBeenTriggered = false;
		waitingForDialogue = false;

		GHOUL_MELEE_PROPERTIES.owner = eID;
	}

	void Update(Entity eID, float dt) {
		if (ReplayManager::Get().IsPlaying()) return;
		if (eID != ghoulEntity || ghoulEntity == INVALID_ENTITY) {
			return; 
		}
		Registry& registry = Registry::Get();

		if (!registry.Has_Component<Movement::MovementComponent>(eID) ||
			!registry.Has_Component<TransformComponent>(eID) ||
			!registry.Has_Component<HealthComponent>(eID) ||
			!registry.Has_Component<RenderComponent>(eID))
		{
			// missing required components
			return;
		}
		
		
		UpdateTimers(dt, ghoulRuntime);

		unsigned int animState = 0;


		HealthComponent& health = registry.Get_Component<HealthComponent>(eID);
		GuiSystem& gui = GuiSystem::Get();
		UIGroup& stats = gui.GetGroup("BossHP");
		int bossStatsID = stats.groupID;
		int level3DialogueID = gui.GetGroup("Level 3 Dialogue").groupID;
		int endDialogueID = gui.GetGroup("End Dialogue").groupID;

		// death handling
		if (health.currhp <= 0.f) {
			// passaway
			if (ghoulRuntime.state != GHOUL_DEAD) {
				ghoulRuntime.state = GhoulState::GHOUL_DEAD;
				ghoulRuntime.stateTime = 0.f;
			}
			animState = 7;
			registry.Get_Component<RenderComponent>(eID).GetSpriteSheet().SetState(animState);
			registry.Get_Component<Movement::MovementComponent>(eID).ResetFrameIntent();

			if (ghoulRuntime.stateTime >= 2.f) {
				// start end dialogue
				if (!hasDeathDialogueBeenTriggered)
				{
					hasDeathDialogueBeenTriggered = true;
					waitingForDialogue = true;
					gui.SetDialogueGate(true);
					gui.ApplyGroupVisibilityOverrides(gui, endDialogueID, true);
				}
				if (waitingForDialogue)
				{
					const bool endDialogueStillVisible = IsAnyDialogueVisibleInGroup(gui, endDialogueID);
					if (!endDialogueStillVisible)
					{
						gui.SetDialogueGate(false);
						gui.ApplyGroupVisibilityOverrides(gui, endDialogueID, false);
						waitingForDialogue = false;
						SceneManager::Get().QueueLoadScene(FilePath::SAVES_FOLDER + "Victory scene.json");
						SceneManager::Get().SetTransitionTime(2.f);
					}
				}
			}
			gui.ApplyGroupVisibilityOverrides(gui, bossStatsID, false);

			return;
		}

		Movement::MovementComponent& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);

		// barrier needs updating
		if (ghoulRuntime.hasBarrier != HasBarrier(eID))
		{
			ghoulRuntime.hasBarrier = HasBarrier(eID);

			if (ghoulRuntime.hasBarrier) // gained barrier (?) should not need to do anyth here
			{
				
			}
			else // lost barrier
			{
				ghoulRuntime.barrierCooldown = GHOUL_BARRIER_COOLDOWN;
				health.isVulnerable = true;
				health.willHit = true;
			}
		}

		// try spawn barrier if can, only after phase 1
		if (ghoulRuntime.phase > 1														// after phase 1
			&& (ghoulRuntime.state == GHOUL_IDLE || ghoulRuntime.state == GHOUL_MOVING)	// neutral states
			&& !ghoulRuntime.hasBarrier && ghoulRuntime.barrierCooldown <= 0.f			// barrier ready to spawn
			&& ghoulRuntime.state != GHOUL_DEAD											// alive lol
			&& ghoulRuntime.state != GHOUL_STAGGERED									// not stunned
			)
		{
			// spawn barrier
			ghoulRuntime.hasBarrier = true;

			Registry::Get().Remove_Component<StatusEffectComponent>(eID); // remove statuses if have, since barrier should cleanse it

			CreateBarrier(eID, GetBarrierElement(ghoulRuntime.barrierElem));

			ghoulRuntime.state = GhoulState::GHOUL_BARRIER;
			ghoulRuntime.stateTime = 0.f;
			health.isVulnerable = false;
		}

		// override with stagger
		else if (CombatSystem::Get().IsCombatantStaggered(eID) && !ghoulRuntime.hasBarrier) // staggered without barrier
		{
			// stagger this fella
			ghoulRuntime.staggerCount++;
			CombatSystem::Get().SetCombatantStagger(eID, false);

			if (ghoulRuntime.staggerCount >= 3) {
				// after 3 hits, stagger ghoul
				ghoulRuntime.staggerCount = 0;
				ghoulRuntime.state = GhoulState::GHOUL_STAGGERED;
				ghoulRuntime.stateTime = 0.f;
			}
		}

		// distance n direction to player
		Vec2 playerPos = GetPos(targetPlayer);
		Vec2 selfPos = GetPos(eID);

		Vec2 directionToPlayer = { playerPos.x - selfPos.x, playerPos.y - selfPos.y };
		bool faceRight = directionToPlayer.x >= 0.f;

		// squared distance
		float dPlayerSq = DistSq(selfPos, playerPos);

		// Camera logic, if player near then make it between both, also trigger start combat
		{
			BehaviourComponent& behaviourComp = registry.Get_Component<BehaviourComponent>(eID);
			if (behaviourComp.HasScript("CamTarget"))
			{
				if (dPlayerSq >= 1000.f * 1000.f) {
					behaviourComp.RemoveScript("CamTarget");
				}
			}
			if (dPlayerSq < 1000.f * 1000.f) {
				behaviourComp.AddScript("CamTarget");

				if (!hasStartingDialogueBeenTriggered)
				{
					hasStartingDialogueBeenTriggered = true;
					waitingForDialogue = true;

					gui.SetDialogueGate(true);
					gui.ApplyGroupVisibilityOverrides(gui, level3DialogueID, true);
				}

				if (waitingForDialogue)
				{
					const bool level3DialogueStillVisible = IsAnyDialogueVisibleInGroup(gui, level3DialogueID);

					if (!level3DialogueStillVisible)
					{
						gui.SetDialogueGate(false);
						gui.ApplyGroupVisibilityOverrides(gui, level3DialogueID, false);
						waitingForDialogue = false;
					}
				}

				if (!waitingForDialogue && !inCombat)
				{
					inCombat = true;
					animState = 6;
					ghoulRuntime.state = GhoulState::GHOUL_TRANSFORM;
					ghoulRuntime.stateTime = 0.f;
					health.willHit = false;
					health.isVulnerable = false;
					gui.ApplyGroupVisibilityOverrides(gui, bossStatsID, true);
				}
			}
		}

		// combat not started
		if (!inCombat) {
			animState = 5; // pre transform
			moveComp.ResetFrameIntent();
			registry.Get_Component<RenderComponent>(eID).GetSpriteSheet().SetState(animState);
			return;
		}
		else if (ghoulRuntime.state == GHOUL_DEAD)
		{
			moveComp.ResetFrameIntent();
			animState = 7;
		}
		else if (ghoulRuntime.state == GhoulState::GHOUL_TRANSFORM) // combat starting!
		{
			moveComp.ResetFrameIntent();
			animState = 6;
			if (health.currhp < health.maxhp)	// simulate healthbar fill up before combat starts
			{
				health.currhp += 3.f;
			}
			else
			{
				health.currhp = health.maxhp;
			}

			if (ghoulRuntime.stateTime >= 2.f) // after transform animation, start combat properly
			{
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
				health.willHit = true;
				health.isVulnerable = true;
			}
		}
		else if (ghoulRuntime.state == GhoulState::GHOUL_STAGGERED) // staggered
		{
			moveComp.ResetFrameIntent();
			animState = 4;
			if (ghoulRuntime.stateTime >= GHOUL_STAGGER_DURATION) {
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
			}
		}
		else if (ghoulRuntime.state == GhoulState::GHOUL_BARRIER) // barrier
		{
			moveComp.ResetFrameIntent();
			animState = 3;
			if (ghoulRuntime.stateTime >= GHOUL_BARRIER_CASTTIME)
			{
				ghoulRuntime.state = GhoulState::GHOUL_IDLE;
				ghoulRuntime.stateTime = 0.f;
			}
		}
		else // alive!
		{
			switch (ghoulRuntime.phase)
			{
			case 1:
			{
				Phase1Logic(eID, moveComp, selfPos, directionToPlayer, animState, dPlayerSq, faceRight);

				if (health.currhp <= health.maxhp * 0.75f) {
					health.currhp = health.maxhp * 0.75f;
					ghoulRuntime.phase = 2;
					ghoulRuntime.state = GhoulState::GHOUL_IDLE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case 1

			case 2:
			{
				Phase2Logic(eID, moveComp, selfPos, directionToPlayer, animState, dPlayerSq, faceRight);

				if (health.currhp <= health.maxhp * 0.4f) {
					health.currhp = health.maxhp * 0.4f;
					ghoulRuntime.phase = 3;
					ghoulRuntime.state = GhoulState::GHOUL_IDLE;
					ghoulRuntime.stateTime = 0.f;
				}
			}
			break; // end case 2

			case 3:
			{
				Phase3Logic(eID, moveComp, selfPos, directionToPlayer, animState, dPlayerSq, faceRight);
			}
			break; // end case 3
			default:
				break;

			}
			// end switch phase
		}


		RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(eID);

		Sprite::SpriteSheet& spriteSheet = renderComp.GetSpriteSheet();


		// always face player
		spriteSheet.Sprite().SetFlipX(faceRight);
		spriteSheet.SetState(animState);


		// hp bar
		{
			// value
			Vec2 v0, v1, v2, v3;
			// Get background quad
			gui.GetDisplayQuad(stats.memberIDs[0], v0, v1, v2, v3);
			// Calculate position for new quad
			float amount = health.currhp / health.maxhp;

			Vec2 newV1 = v0 + (v1 - v0) * amount;   // bottom right moves left
			Vec2 newV2 = v3 + (v2 - v3) * amount;   // top right moves left

			gui.SetDisplayQuad(stats.memberIDs[1], v0, newV1, newV2, v3);

			// phase
			gui.SetDisplaySheetState(stats.memberIDs[2], ghoulRuntime.phase - 1);
		}
		// end update for ghoul
	}

	void End(Entity eID) {
		std::cout << "Spirit script ended for entity " << eID << std::endl;
		ghoulRuntime = GhoulRuntime();
		ghoulEntity = INVALID_ENTITY;
		GHOUL_MELEE_PROPERTIES.owner = INVALID_ENTITY;
		if (HasBarrier(eID)) {
			g_elemBarriers[eID].active = false;
		}
		CombatSystem::Get().RemoveCombatant(eID);
		inCombat = false;
	}
}


void EnemySystem::Initialize()
{
	BehaviourFCT enemy;
	enemy.Init = [](Entity e) { EnemyAI::Start(e); };
	enemy.Update = [](Entity e, float dt) { EnemyAI::Update(e, dt); };
	enemy.End = [](Entity e) { EnemyAI::End(e); };
	LogicSystem::Get().RegisterScript("ManfredEnemy", enemy);

	BehaviourFCT spirit;
	spirit.Init = [](Entity e) { SpiritScript::Start(e); };
	spirit.Update = [](Entity e, float dt) { SpiritScript::Update(e, dt); };
	spirit.End = [](Entity e) { SpiritScript::End(e); };
	LogicSystem::Get().RegisterScript("SpiritScript", spirit);

	BehaviourFCT obstacle;
	obstacle.Init = [](Entity e) { ObstacleScript::Start(e); };
	obstacle.Update = [](Entity e, float dt) { ObstacleScript::Update(e, dt); };
	obstacle.End = [](Entity e) { ObstacleScript::End(e); };
	LogicSystem::Get().RegisterScript("ObstacleScript", obstacle);

	BehaviourFCT ghoul;
	ghoul.Init = [](Entity e) { GhoulScript::Start(e); };
	ghoul.Update = [](Entity e, float dt) { GhoulScript::Update(e, dt); };
	ghoul.End = [](Entity e) { GhoulScript::End(e); };
	LogicSystem::Get().RegisterScript("GhoulScript", ghoul);

	SceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([](const SceneLoadEvent& e) {
		// on scene load, wipe spirits and assign target player
		g_spirits.clear();
		g_elemBarriers.clear();
		g_obstacles.clear();

		if (e.sceneType == SceneType::ST_GAME) {
			AssignTarget();
		}
		else {
			ClearTarget();
		}
		});

}

void EnemySystem::Update(float dt)
{
	(void)dt;
	SceneManager& sm = SceneManager::Get();
	// check if game paused
	if (sm.IsPaused())
	{
		return;  // Don't update enemy movement or footsteps when paused
	}

	for (auto& [entity, comp] : m_registry.Get_ComponentMap<TagComponent>())
	{
		if (!m_registry.Has_Component<TransformComponent>(entity))
			continue;
		if (comp.tag == T_ENEMY)
		{
			// THIS IS AN ENEMY, DO ENEMY THINGS
			//ChasePlayer(entity, EnemyAI::FindPlayer(), dt);
			//RoamAround(entity, dt);
			//ReturnToBase(entity, dt);
		}
	}

	UpdateBarriers();
}

void EnemySystem::Shutdown()
{
	EventBus::Get().Unsubscribe<SceneLoadEvent>(SceneSubscriberID);
}