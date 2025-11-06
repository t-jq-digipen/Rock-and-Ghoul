#pragma once
#include "EnemySystem.h"
#include "MovementSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Remath.h"
#include "BehaviourFCT.h"
#include "LogicSystem.h"
#include "GameScripts.h"
#include "CoreEngine.h"

// Temporary const values
static constexpr float DETECT_RADIUS = 300.f; // detection radius
static constexpr float LOSE_RADIUS = 360.f;  // hysteresis
static constexpr float STOP_DISTANCE = 8.f;    // arrive radius
static constexpr float CHASE_SPEED = 300.f;  // units/sec

// Roam constants
static constexpr float ROAM_SINE_SPEED = 2.f; // "speed" (radians/sec, matches time*speed)
static constexpr float ROAM_SPEED_MAX= 200.f;   // cap velocity (units/sec)
static constexpr float ROAM_AMPLITUDE = 100.f;   // how far up/down when roaming
static constexpr float EPS = 0.5f; // prevents jitter at the roaming boundary

// Return constants
static constexpr float RETURN_SPEED = 600.f; // units/sec

// Internal enemy state check (no ECS included!!!)
struct EnemyAIState
{
	EnemyState state{ E_ROAMING };
	float baseX{ 0.f };
	float baseY{ 0.f };
	float time{ 0.f };
	bool basePosInitialized{ false };
	bool movingUp{ true };
};

static std::unordered_map<Entity, EnemyAIState> e_state;

// Helper to ensure MovementComponent exists and is configured
static inline Movement::MovementComponent& EnsureMovement(Entity eID)
{
	//if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID))
	//{
	//	Registry::Get().Add_Component<Movement::MovementComponent>(eID, Movement::MovementComponent{});
	//	Registry::Get().Get_Component<Movement::MovementComponent>(eID).SetMoveType(Movement::MOVE_VELOCITY); // ensure movement by velocity
	//}
	auto& mv = Registry::Get().Get_Component<Movement::MovementComponent>(eID);
	mv.SetMoveType(Movement::MOVE_POSITION);
	return mv;
}

// Helper to set velocity for entity (normalizes direction internally)
static inline void SetVelocity(Entity eID, Vec2 dir, float speed, bool isMoving)
{
	auto& mv = EnsureMovement(eID);

	// Normalize defensively
	const float m2 = RG_LengthSquared(dir);
	if (isMoving && m2 > 0.f)
	{
		const float inv = 1.f / RG_Sqrt(m2);
		dir.x *= inv; dir.y *= inv;
	}
	else if (!isMoving || m2 == 0.f)
	{
		dir = { 0.f, 0.f };
		speed = 0.f;
		isMoving = false;
	}

	mv.SetInputDirection(Movement::MOVE_NONE);
	mv.SetMovementDirection(dir);
	mv.SetMoveSpeed(speed);
	mv.SetIsMoving(isMoving);
}

// Enemy chasing player (E_CHASING)
// Movement implemented via MovementComponent
void ChasePlayer(Entity selectID, Entity targetID, float& dt)
{
	if (dt <= 0.f || targetID == INVALID_ENTITY) return;
	if (!Registry::Get().Has_Component<TransformComponent>(selectID) ||
		!Registry::Get().Has_Component<TransformComponent>(targetID))
		return;

	// Only continue if enemy state is E_CHASING
	auto st = e_state.find(selectID);
	if (st == e_state.end() || st->second.state != E_CHASING) {
		return;
	}

	auto& eTrans = Registry::Get().Get_Component<TransformComponent>(selectID);
	auto& pTrans = Registry::Get().Get_Component<TransformComponent>(targetID);

	const float dx = pTrans.pos.x - eTrans.pos.x;
	const float dy = pTrans.pos.y - eTrans.pos.y;
	const float dist = dx * dx + dy * dy;
	const float stopDist = STOP_DISTANCE * STOP_DISTANCE;

	if (dist <= stopDist) {
		// Stop movement with MovementComponent
		SetVelocity(selectID, { 0.f, 0.f }, 0.f, false);
		return;
	}

	// Determine movement with MovementComponent
	SetVelocity(selectID, { dx, dy }, CHASE_SPEED, true);

	// Set facing
	// eTrans.rot = static_cast<float>(std::atan2(dy, dx) * (180.0 / RG_PI));
}

// Enemy roaming around (E_ROAMING)
// Movement implemented via MovementComponent
void RoamAround(Entity eID, float& dt)
{
	if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID)) return;

	// Only continue if enemy state is E_ROAMING
	auto& st = e_state[eID];
	if (st.state != E_ROAMING) return;

	// Ensure can push velocity intent
	EnsureMovement(eID);

	st.time += dt;

	const float omega = ROAM_SINE_SPEED;
	const float amp = ROAM_AMPLITUDE;
	const float vy = amp * omega * std::cos(st.time * omega);
	const float absVy = std::fabs(vy);

	// if close enough to boundary, stop this frame
	if (absVy < EPS)
	{
		SetVelocity(eID, { 0.f, 0.f }, 0.f, false);
		return;
	}

	const float dirY = (vy > 0.f) ? 1.f : -1.f;
	const float appliedSpeed = std::min(absVy, ROAM_SPEED_MAX);

	// Set movement velocity via MovementComponent
	SetVelocity(eID, { 0.f, dirY }, appliedSpeed, true);
}

// Enemy returning to base position (E_RETURNING)
// Movement implemented via MovementComponent
void ReturnToBase(Entity eID, float& dt)
{
	if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID)) return;

	auto it = e_state.find(eID);
	if (it == e_state.end() || it->second.state != E_RETURNING) return;

	auto& st = it->second;
	auto& tr = Registry::Get().Get_Component<TransformComponent>(eID);

	const float dx = st.baseX - tr.pos.x;
	const float dy = st.baseY - tr.pos.y;
	const float distToBase = dx * dx + dy * dy;

	// When enemy arrives back to base position
	if (distToBase <= EPS * EPS) {
		SetVelocity(eID, { 0.f,0.f }, 0.f, /*moving=*/false);
		return;
	}

	const float dist = std::sqrt(distToBase);
	const float maxSpeedThisFrame = dist / std::max(dt, 1e-6f);
	const float appliedSpeed = std::min(RETURN_SPEED, maxSpeedThisFrame);

	SetVelocity(eID, { dx, dy }, appliedSpeed, /*moving=*/true);
}

namespace EnemyAI
{
	// Helper to find player entity
	static inline Entity FindPlayer()
	{
		for (auto& [entity, comp] : Registry::Get().Get_ComponentMap<TagComponent>())
		{
			if (comp.tag == T_PLAYER && entity == 1 && Registry::Get().Has_Component<Movement::MovementComponent>(entity))
			{
				return entity;
			}
		}
		return INVALID_ENTITY;
	}

	inline void Start(Entity eID)
	{
		auto& st = e_state[eID];
		if (Registry::Get().Has_Component<TransformComponent>(eID))
		{
			auto& tr = Registry::Get().Get_Component<TransformComponent>(eID);
			st.baseX = tr.pos.x;
			st.baseY = tr.pos.y;
			st.basePosInitialized = true;
			st.time = 0.f;
			st.state = E_ROAMING;
		}
		EnsureMovement(eID);
		std::cout << "Enemy script started" << std::endl;
	}

	inline void Update(Entity eID, float dt)
	{
		(void)dt;
		if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID)) return;

		auto& tr = Registry::Get().Get_Component<TransformComponent>(eID);
		auto& st = e_state[eID];

		if (!st.basePosInitialized)
		{
			st.baseX = tr.pos.x;
			st.baseY = tr.pos.y;
			st.basePosInitialized = true;
			st.time = 0.f;
			st.state = E_ROAMING;
		}

		// Get player and distance between enemy and player
		const Entity player = FindPlayer();
		float dist = std::numeric_limits<float>::infinity();
		if (player != INVALID_ENTITY)
		{
			auto& p = Registry::Get().Get_Component<TransformComponent>(player);
			dist = RG_DistanceSquared(tr.pos, p.pos);
		}

		const float detectRad = DETECT_RADIUS * DETECT_RADIUS;	// check if player is within detection radius
		const float loseRad = LOSE_RADIUS * LOSE_RADIUS;		// check if player is outside of lose radius

		// For E_RETURNING
		Vec2 basePos = { st.baseX, st.baseY };
		Vec2 currPos = { tr.pos.x, tr.pos.y };
		const float distToBase = RG_DistanceSquared(currPos, basePos);

		// Enemy state transitions
		switch (st.state)
		{
			case E_ROAMING:
				// std::cout << "Ghoul is ROAMING!" << std::endl;
				if (player != INVALID_ENTITY && dist <= detectRad)
				{
					st.state = E_CHASING;
				}
				break;
			case E_CHASING:
				std::cout << "Ghoul is CHASING YOU!" << std::endl;
				if (player == INVALID_ENTITY || dist > loseRad)
				{
					st.state = E_RETURNING;
				}
				break;
			case E_RETURNING:
				std::cout << "Ghoul is RETURNING..." << std::endl;
				if (player != INVALID_ENTITY && dist <= detectRad)
				{
					st.state = E_CHASING;
					break;
				}
				if (distToBase <= EPS * EPS)
				{
					st.state = E_ROAMING;
				}
				break;
			default:
				std::cout << "Ghoul is IDLING." << std::endl;
				break;
		}
	}

	inline void End(Entity eID)
	{
		e_state.erase(eID);
		if (Registry::Get().Has_Component<Movement::MovementComponent>(eID))
		{
			Registry::Get().Get_Component<Movement::MovementComponent>(eID).ResetFrameIntent();
		}
		std::cout << "Enemy script ended" << std::endl;
	}
}

void EnemySystem::Initialize()
{
	{
		BehaviourFCT enemy;
		enemy.Init = [](Entity e) { EnemyAI::Start(e); };
		enemy.Update = [](Entity e, float dt) { EnemyAI::Update(e, dt); };
		enemy.End = [](Entity e) { EnemyAI::End(e); };
		LogicSystem::Get().RegisterScript("PatrolEnemy", enemy, ScriptID::GHOUL);
	}
}

void EnemySystem::Update(float dt)
{
	Entity player = INVALID_ENTITY;

	// check if game paused
	if (GSM_IsGamePaused())
	{
		return;  // Don't update enemy movement or footsteps when paused
	}	

	for (auto& [entity, comp] : m_registry.Get_ComponentMap<TagComponent>())
	{
		if (!m_registry.Has_Component<TransformComponent>(entity))
			continue;
		if (comp.tag == T_PLAYER && entity == 1)
		{
			player = entity;
		}
		if (comp.tag == T_ENEMY)
		{
			// THIS IS AN ENEMY, DO ENEMY THINGS
			ChasePlayer(entity, player, dt);
			RoamAround(entity, dt);
			ReturnToBase(entity, dt);
		}
	}
}

void EnemySystem::Shutdown()
{

}