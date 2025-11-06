/* ========================================================
 * @file	    MovementSystem.cpp
 * @author	    Manfred Sim | manfred.sim@digipen.edu
 * @date	    30 OCT 2025
 *
 * @brief	Movement system for ECS
 *
 * This file implements the update order of the various
 * parts in the movement component, with reference to other
 * components such as Transform (for movement by position) and 
 * Physics (for movement by velocity).
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "MovementSystem.h"
#include "PhysicsSystem.h"
#include "CoreEngine.h"

void MovementSystem::Initialize()
{

}

void MovementSystem::Update(float dt)
{
	(void)dt;

	// Define movement logic (player input handled in PlayerSystem)

	for (auto& [entID, moveComp] : m_registry.Get_ComponentMap<Movement::MovementComponent>())
	{
		// Check if entity is moving
		const Vec2 dir = moveComp.GetMovementDirection();
		if (!moveComp.GetIsMoving() || (dir.x == 0.f && dir.y == 0.f))
		{
			moveComp.ResetFrameIntent();
			continue;
		}

		// Get entity speed (different for each entity)
		const float speed = moveComp.GetMoveSpeed();

		// Define if moving by position or velocity
		switch (moveComp.GetMoveType())
		{
			case Movement::MOVE_POSITION:
			{
				if (!m_registry.Has_Component<TransformComponent>(entID)) break;
				auto& trans = m_registry.Get_Component<TransformComponent>(entID);
				trans.pos.x += dir.x * speed * dt;
				trans.pos.y += dir.y * speed * dt;
				break;
			}
			case Movement::MOVE_VELOCITY:
			{
				if (!m_registry.Has_Component<Physics::PhysicsComponent>(entID)) break;
				auto& phys = m_registry.Get_Component<Physics::PhysicsComponent>(entID);
				Vec2 vel{ dir.x * speed, dir.y * speed };
				phys.ApplyLinearImpulse(vel);
				if (!moveComp.GetIsMoving())
				{
					phys.Set_Vel({ 0.f, 0.f });
				}
				break;
			}
			default:
				break;
		}

		// Reset move component to 0 after each frame
		moveComp.ResetFrameIntent();
	}
}

void MovementSystem::Shutdown()
{
	std::cout << "MovementSystem Shutdown!" << std::endl;
}