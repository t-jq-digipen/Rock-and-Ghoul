#pragma once
#include "EnemySystem.h"
#include "Registry.h"
#include "Component.h"
#include "Remath.h"
#include "BehaviourFCT.h"
#include "LogicSystem.h"
#include "GameScripts.h"
#include "CoreEngine.h"

void ChasePlayer(Entity selectID, Entity targetID, float& dt)
{
	if (!Registry::Get().Has_Component<TransformComponent>(selectID) ||
		!Registry::Get().Has_Component<TransformComponent>(targetID)) return;
	auto& eTrans = Registry::Get().Get_Component<TransformComponent>(selectID);
	auto& pTrans = Registry::Get().Get_Component<TransformComponent>(targetID);

	const float ex = eTrans.pos.x;
	const float ey = eTrans.pos.y;
	const float tx = pTrans.pos.x;
	const float ty = pTrans.pos.y;

	const float dx = tx - ex;
	const float dy = ty - ey;

	const double dist = std::sqrt(static_cast<double>(dx) * dx + static_cast<double>(dy) * dy);
	constexpr float stopDistance = 8.0f;
	if (dist <= static_cast<double>(stopDistance) || dist == 0.0) return;

	const float invLen = static_cast<float>(1.0 / dist);
	const float dirX = dx * invLen;
	const float dirY = dy * invLen;

	constexpr float speed = 200.0f; // units/sec
	const float maxStep = speed * dt;
	const float remaining = static_cast<float>(dist) - stopDistance;
	const float step = std::min(maxStep, remaining);

	eTrans.pos.x = ex + dirX * step;
	eTrans.pos.y = ey + dirY * step;

	// Face target (degrees). Adjust if your transform expects radians.
	eTrans.rot = static_cast<float>(std::atan2(dirY, dirX) * (180.0 / RG_PI));
}

namespace EnemyAI
{
	inline void Start(Entity eID)
	{
		std::cout << "Enemy script started" << std::endl;
	}

	inline void Update(Entity eID, float dt)
	{
       
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
	if (GSM_IsGamePaused()) {
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
		}

	}
}

void EnemySystem::Shutdown()
{

}