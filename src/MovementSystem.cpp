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

	for (auto& [entID, moveComp] : m_registry.Get_ComponentMap<Movement::MovementComponent>())
	{
		// Get respective input scheme
		ControlScheme scheme = CS_WASD;
		if (m_registry.Has_Component<InputComponent>(entID))
		{
			scheme = m_registry.Get_Component<InputComponent>(entID).scheme;
		}

		Vec2 dir = ReadDirFromScheme(scheme);
		const bool norm = Normalize(dir);

		moveComp.SetIsMoving(norm);
		moveComp.SetMoveDirection(VelToDir(norm ? dir : Vec2{ 0.f, 0.f }));

		if (!norm) continue;

		const float speed = moveComp.GetMoveSpeed();

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
				phys.AddForce(dir, speed, 0.f);
				break;
			}
			default:
				break;
		}
	}
}

void MovementSystem::Shutdown()
{
	std::cout << "MovementSystem Shutdown!" << std::endl;
}