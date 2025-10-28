#include "MovementSystem.h"

void MovementSystem::Initialize()
{

}

void MovementSystem::Update(float dt)
{
	(void)dt;
	for (auto& [entID, phyComp] : m_registry.Get_ComponentMap<Movement::MovementComponent>())
	{

	}
}

void MovementSystem::Shutdown()
{
	std::cout << "MovementSystem Shutdown!" << std::endl;
}