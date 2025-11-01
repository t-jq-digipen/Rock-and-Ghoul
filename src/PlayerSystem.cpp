/* ========================================================
 * @file	    PlayerSystem.cpp
 * @author	    Tan Jun Quan	   | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi		   | yasmin.t@digipen.edu
 * @date	    31 OCT 2025
 *
 * @brief	Player System for handling player behaviour.
 *
 * This file implements the movement controls of a player
 * entity based on forces, has a dash, and switches player
 * states accordingly.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "PlayerSystem.h"
#include "Registry.h"


 // CONSTANTS for responsive, non-tunneling movement, can adjust
constexpr float MOVE_FORCE = 1000.f;    // Acceleration force
constexpr float MAX_SPEED = 200.f;      // Top speed cap
constexpr float DASH_IMPULSE = 1000.f;   // Dash boost

void PlayerSystem::WASD_Movement(Entity eID, float& dt)
{
	(void)dt;
	auto& physics = m_registry.Get_Component<Physics::PhysicsComponent>(eID);

	if (!m_registry.Has_Component<InputComponent>(eID)) {
		return; // No input component, skip movement
	}
	auto& input = m_registry.Get_Component<InputComponent>(eID);

	// Ensure max speed is set (prevents tunneling)
	if (physics.GetMaxSpeed() < MAX_SPEED) {
		physics.SetMaxSpeed(MAX_SPEED);
	}
	// Clear forces every frame so that player instantly switches directions
	physics.ClearForces();

	// Track input direction for dash
	int vert = 0;
	int hor = 0;

	// Handle movement based on control scheme
	switch (input.scheme) {
		case CS_WASD :
		if (Input::IsKeyHeld(GLFW_KEY_W)) {
			physics.AddForce({ 0, 1 }, MOVE_FORCE, 0.f);
			vert += 1;
		}
		if (Input::IsKeyHeld(GLFW_KEY_S)) {
			physics.AddForce({ 0, -1 }, MOVE_FORCE, 0.f);
			vert -= 1;
		}
		if (Input::IsKeyHeld(GLFW_KEY_A)) {
			physics.AddForce({ -1, 0 }, MOVE_FORCE, 0.f);
			hor -= 1;
		}
		if (Input::IsKeyHeld(GLFW_KEY_D)) {
			physics.AddForce({ 1, 0 }, MOVE_FORCE, 0.f);
			hor += 1;
		}
		break;

		case CS_IJKL:
			if (Input::IsKeyHeld(GLFW_KEY_I)) {
				physics.AddForce({ 0, 1 }, MOVE_FORCE, 0.f);
				vert += 1;
			}
			if (Input::IsKeyHeld(GLFW_KEY_K)) {
				physics.AddForce({ 0, -1 }, MOVE_FORCE, 0.f);
				vert -= 1;
			}
			if (Input::IsKeyHeld(GLFW_KEY_J)) {
				physics.AddForce({ -1, 0 }, MOVE_FORCE, 0.f);
				hor -= 1;
			}
			if (Input::IsKeyHeld(GLFW_KEY_L)) {
				physics.AddForce({ 1, 0 }, MOVE_FORCE, 0.f);
				hor += 1;
			}
			// Update animation state
			if (m_registry.Has_Component<Animator::AnimatorComponent>(eID))
			{
				bool isMoving = (Input::IsKeyHeld(GLFW_KEY_I) || Input::IsKeyHeld(GLFW_KEY_J) ||
					Input::IsKeyHeld(GLFW_KEY_K) || Input::IsKeyHeld(GLFW_KEY_L));
				m_registry.Get_Component<Animator::AnimatorComponent>(eID).SetState(
					isMoving ? PS_WALK : PS_IDLE
				);
			}
			break;
		case CS_ARROWS:
			if (Input::IsKeyHeld(GLFW_KEY_UP)) {
				physics.AddForce({ 0, 1 }, MOVE_FORCE, 0.f);
				vert += 1;
			}
			if (Input::IsKeyHeld(GLFW_KEY_DOWN)) {
				physics.AddForce({ 0, -1 }, MOVE_FORCE, 0.f);
				vert -= 1;
			}
			if (Input::IsKeyHeld(GLFW_KEY_LEFT)) {
				physics.AddForce({ -1, 0 }, MOVE_FORCE, 0.f);
				hor -= 1;
			}
			if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) {
				physics.AddForce({ 1, 0 }, MOVE_FORCE, 0.f);
				hor += 1;
			}
			break;
	}
	// DASH - Working impulse
	if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
		Vec2 dashDir{ (float)hor, (float)vert };

		// If no input, use current velocity
		if (hor == 0 && vert == 0) {
			dashDir = physics.Get_Vel();
		}

		if (RG_LengthSquared(dashDir) > 0.01f) {
			dashDir = RG_Normalize(dashDir);
			Vec2 dashImpulse = RG_Scale(dashDir, DASH_IMPULSE);
			physics.AddImpulse(dashImpulse);
		}
	}
}

void PlayerSystem::Initialize()
{

}

void PlayerSystem::Update(float dt)
{
	for (auto& [entity, comp] : m_registry.Get_ComponentMap<TagComponent>())
	{
		if (!m_registry.Has_Component<TransformComponent>(entity))
			continue;
		if (comp.tag == T_PLAYER)
		{
			// THIS IS A PLAYER, DO PLAYER THINGS
			WASD_Movement(entity, dt);

		}
	}
}
