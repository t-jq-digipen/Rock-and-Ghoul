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
 * Copyright � 2025 DigiPen, All rights reserved.
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
#include "JsonLoad.h"
#include <unordered_map>
#include "CollisionManager.h"
#include "PlayerSystem.h"
#include "CombatSystem.h"

static constexpr float DETECT_RADIUS = 300.f;
static constexpr float LOSE_RADIUS = 360.f;
static constexpr float STOP_DISTANCE = 8.f;

static constexpr float ROAM_SINE_SPEED = 2.f;
static constexpr float ROAM_SPEED_MAX = 200.f;
static constexpr float ROAM_AMPLITUDE = 100.f;
static constexpr float ROAM_EPS = 0.5f;

static constexpr float CHASE_SPEED = 300.f;
static constexpr float RETURN_SPEED = 600.f;

static constexpr float MELEE_RANGE = 150.f;
static constexpr float MELEE_COOLDOWN = 0.8f;
static constexpr float MELEE_WINDUP = 0.65f;
static constexpr float MELEE_DURATION = 1.f;
static constexpr float MELEE_DAMAGE = 10.f;


static constexpr float SHOOT_RANGE = 240.f;
static constexpr float SHOOT_COOLDOWN = 1.2f;
static constexpr float PROJECTILE_SPEED = 600.f;

static constexpr float DEATH_DURATION = 1.0f;
static constexpr int ANIM_DEATH_STATE = 2;

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
		// Check if enemy is stunned
		if (Registry::Get().Has_Component<StatusEffectComponent>(eID)) {
			auto& statusEffect = Registry::Get().Get_Component<StatusEffectComponent>(eID);
			if (statusEffect.isStunned) {
				// Stop movement while stunned
				SetVelocity(eID, { 0.f, 0.f }, 0.f, false);
				return;
			}
		}

		auto& tr = Registry::Get().Get_Component<TransformComponent>(eID);
		auto& st = e_state[eID];

		if (!st.basePosInitialized)
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

size_t SceneSubscriberID = -1;

namespace SpiritMeleeAttack {


	struct AttackProperties {
		size_t subscriberID{};
		Entity owner{};
	};

	void HitPlayer(Entity owner, Entity player) {
		// check if player is shielded or hit already
		// bruh need to link to shield oops YASMINHELP


		// knockback
		// direction
		auto& ownerTransform = Registry::Get().Get_Component<TransformComponent>(owner);
		auto& playerTransform = Registry::Get().Get_Component<TransformComponent>(player);

		bool left = playerTransform.pos.x < ownerTransform.pos.x;
		RockScript::Knockback(player, left, MELEE_DAMAGE);

		auto& physicsComp = Registry::Get().Get_Component<Physics::PhysicsComponent>(player);
	}

	std::unordered_map<Entity, AttackProperties> g_meleeProperties;

	inline void Start(Entity eID) {
		g_meleeProperties[eID].subscriberID = EventBus::Get().Subscribe<TriggerEvent>([eID](const TriggerEvent& e) {
			// idk how to name it yet lol
			// if (e.eventName != "SpiritMeleeHit") return;

			if (e.other != targetPlayer || e.trigger != eID) return;

			HitPlayer(g_meleeProperties[eID].owner, targetPlayer);
			Registry::Get().Remove_Component<ColliderComponent>(e.trigger);
			}
		);
	}
	inline void Update(Entity, float) {

	}
	inline void End(Entity eID) {

		EventBus::Get().Unsubscribe<TriggerEvent>(g_meleeProperties[eID].subscriberID);
		g_meleeProperties.erase(eID);
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
		g_spirits[eID] = SpiritRuntime();
	}

	inline void Update(Entity eID, float dt) {
		if (targetPlayer == INVALID_ENTITY) {
			// No target player assigned, skipping update for spirit
			return;
		}
		// registry
		Registry& registry = Registry::Get();

		if (!registry.Has_Component<Movement::MovementComponent>(eID) ||
			!registry.Has_Component<TransformComponent>(eID) ||
			!registry.Has_Component<HealthComponent>(eID) ||
			!registry.Has_Component<RenderComponent>(eID)) 
		{
			// missing required components
			return;
		}

		auto& moveComp = registry.Get_Component<Movement::MovementComponent>(eID);
		auto& transComp = registry.Get_Component<TransformComponent>(eID);
		auto& healthComp = registry.Get_Component<HealthComponent>(eID);

		RenderComponent& renderComp = Registry::Get().Get_Component<RenderComponent>(eID);
		Sprite::SpriteSheet& spriteSheet = renderComp.GetSpriteSheet();

		SpiritRuntime& spiritLogic = g_spirits[eID];

		if (healthComp.currhp <= 0.f) {
			// spirit state not dead yet
			if (spiritLogic.state != SpiritState::SPIRIT_DEAD) {
				spiritLogic.state = SpiritState::SPIRIT_DEAD;
				spiritLogic.stateTime = 0.f;
				spriteSheet.SetState(2);
				registry.Add_Component<LifetimeComponent>(eID, LifetimeComponent{ 0.f, DEATH_DURATION });
				moveComp.ResetFrameIntent();
			}
			else // spirit is dead 
			{
				spiritLogic.stateTime += dt;
				spriteSheet.SetState(2);
				//if (spiritLogic.stateTime >= DEATH_DURATION) {
				//	// remove spirit entity
				//	//g_spirits.erase(eID);
				//}
			}
			// no need check the rest bc this fella die alr
			return;
		}

		// get player pos and spirit pos
		Vec2 playerPos = GetPos(targetPlayer);
		Vec2 selfPos = GetPos(eID);

		Vec2 directionToPlayer = { playerPos.x - selfPos.x, playerPos.y - selfPos.y };


		// always face player
		bool faceRight = directionToPlayer.x >= 0.f;
		spriteSheet.Sprite().SetFlipX(faceRight);
		

		//spriteSheet.SetState(animstate);

		if (spiritLogic.state == SpiritState::SPIRIT_IDLE) {

			spriteSheet.SetState(0);
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
			spriteSheet.SetState(0);
			moveComp.ResetFrameIntent();
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveComp.SetIsMoving(true);
			moveComp.SetMovementDirection(moveDir);

			// if close enough, time to smack
			float dPlayerSq = DistSq(selfPos, playerPos);
			float meleeSq = MELEE_RANGE * MELEE_RANGE;
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
			spriteSheet.SetState(1);
			spiritLogic.stateTime += dt;

			if (spiritLogic.stateTime >= MELEE_WINDUP && spiritLogic.willAttack) {
				spiritLogic.willAttack = false;
				// spawn the hitbox
				// spawn prefab here
				Entity meleeHitbox = JsonLoad::LoadEntityFromFile(FilePath::PREFABS_FOLDER + "SpiritMelee.json", selfPos.x, selfPos.y);
				ColliderComponent& colliderComp = Registry::Get().Get_Component<ColliderComponent>(meleeHitbox);
				colliderComp.triggerEvent = "SpiritMeleeHit";
				SpiritMeleeAttack::g_meleeProperties[meleeHitbox].owner = eID;
			}

			if (spiritLogic.stateTime >= MELEE_DURATION) {
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
			spriteSheet.SetState(0);
			Vec2 moveDir = RG_Normalize(directionToPlayer);
			moveDir.x = -moveDir.x;
			moveDir.y = -moveDir.y;
			moveComp.SetIsMoving(true);
			moveComp.SetMovementDirection(moveDir);

			if (spiritLogic.stateTime >= 1.5f) {
				// add chance for range later
				spiritLogic.willAttack = true;

				spiritLogic.stateTime = 0.f;
				spiritLogic.state = SpiritState::SPIRIT_CHASE;
				moveComp.ResetFrameIntent();
			}
		}
		break;
		// end case SPIRIT_ESCAPE

		case SpiritState::SPIRIT_RANGE:
		{
			spiritLogic.stateTime += dt;
			// for now just set back to chase lol it shouldnt go to range yet
			spiritLogic.stateTime = 0.f;
			spiritLogic.state = SpiritState::SPIRIT_CHASE;
		}
		break;
		// end case SPIRIT_RANGE

		case SpiritState::SPIRIT_HIT:
		{
			spriteSheet.SetState(3);
			spiritLogic.stateTime += dt;

			spiritLogic.willAttack = false;
			if (spiritLogic.stateTime >= 0.5f) {
				spiritLogic.stateTime = 0.f;
				spiritLogic.state = SpiritState::SPIRIT_CHASE;
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
		
	}
}


void EnemySystem::Initialize()
{
	BehaviourFCT enemy;
	enemy.Init = [](Entity e) { EnemyAI::Start(e); };
	enemy.Update = [](Entity e, float dt) { EnemyAI::Update(e, dt); };
	enemy.End = [](Entity e) { EnemyAI::End(e); };
	LogicSystem::Get().RegisterScript("ManfredEnemy", enemy, ScriptID::GHOUL);

	BehaviourFCT spirit;
	spirit.Init = [](Entity e) { SpiritScript::Start(e); };
	spirit.Update = [](Entity e, float dt) { SpiritScript::Update(e, dt); };
	spirit.End = [](Entity e) { SpiritScript::End(e); };
	LogicSystem::Get().RegisterScript("SpiritScript", spirit, ScriptID::SPIRIT);

	BehaviourFCT spiritMelee;
	
	spirit.Init = [](Entity e) { SpiritMeleeAttack::Start(e); };
	spirit.Update = [](Entity e, float dt) { SpiritMeleeAttack::Update(e, dt); };
	spirit.End = [](Entity e) { SpiritMeleeAttack::End(e); };
	LogicSystem::Get().RegisterScript("SpiritMeleeScript", spirit, ScriptID::SPIRITMELEE);

	SceneSubscriberID = EventBus::Get().Subscribe<SceneLoadEvent>([](const SceneLoadEvent& e) {
		// on scene load, wipe spirits and assign target player
		g_spirits.clear();
		if (e.sceneType == SceneType::ST_GAME) {
			AssignTarget();
		}
		else {
			ClearTarget();
		}
		});
	// Initialize combat system (handles projectile hits and element effects)
	CombatSystem::Initialize();
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

	// Update element effects via CombatSystem
	CombatSystem::UpdateElementEffects(m_registry, dt);

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
}

void EnemySystem::Shutdown()
{
	EventBus::Get().Unsubscribe<SceneLoadEvent>(SceneSubscriberID);
}