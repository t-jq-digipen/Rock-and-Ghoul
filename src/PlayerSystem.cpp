/* ========================================================
 * @file	    PlayerSystem.cpp
 * @author	    Tan Jun Quan	   | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi		   | yasmin.t@digipen.edu
 * @co-author   Manfred Sim | manfred.sim@digipen.edu
 * @date	    01 NOV 2025
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
#include "Audio.h"
#include "GameStateManager.h"
#include "CoreEngine.h" // to check pause state for music
#include "LogicSystem.h"

 // CONSTANTS for responsive, non-tunneling movement
constexpr float MOVE_FORCE = 1000.f;    // Acceleration force
constexpr float MAX_SPEED = 200.f;      // Top speed cap

void NormalizeDiagonalMovement(float& x, float& y)
{
	const float l2 = x*x + y*y;
	if (l2 <= 0.f)
	{
		x = y = 0.f;
		return;
	}
	const float inv = 1.f / std::sqrt(l2);
	x *= inv; y *= inv;
}

Movement::InputDirection VelToDir(float x, float y)
{
	if (std::abs(y) >= std::abs(x)) return (y > 0.f) ? Movement::MOVE_UP : (y < 0.f ? Movement::MOVE_DOWN : Movement::MOVE_NONE);
	return (x > 0.f) ? Movement::MOVE_RIGHT : (x < 0.f ? Movement::MOVE_LEFT : Movement::MOVE_NONE);
}

void PlayerSystem::WASD_Movement(Entity eID, float& dt)
{
	if (!m_registry.Has_Component<Movement::MovementComponent>(eID)) return;
	auto& moveComp = m_registry.Get_Component<Movement::MovementComponent>(eID);
	auto& physics = m_registry.Get_Component<Physics::PhysicsComponent>(eID);

	if (!m_registry.Has_Component<InputComponent>(eID)) {
		moveComp.SetIsMoving(false);
		moveComp.SetInputDirection(Movement::MOVE_NONE);
		moveComp.SetMovementDirection({ 0.f, 0.f });
		return;
	}

	const auto& input = m_registry.Get_Component<InputComponent>(eID);

	// Get input direction
	int hor = 0, ver = 0;
	switch (input.scheme)
	{
	case CS_WASD:
		if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
		break;
	case CS_IJKL:
		if (Input::IsKeyHeld(GLFW_KEY_I)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_K)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_J)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_L)) hor += 1;
		break;
	case CS_ARROWS:
		if (Input::IsKeyHeld(GLFW_KEY_UP))    ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_DOWN))  ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_LEFT))  hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) hor += 1;
		break;
	default: break;
	}

	// Check if entity has dash component
	bool hasDash = m_registry.Has_Component<DashComponent>(eID);
	DashComponent* dashComp = hasDash ? &m_registry.Get_Component<DashComponent>(eID) : nullptr;

	// Update dash timer
	if (dashComp) {
		dashComp->Update(dt);
	}

	float fx = static_cast<float>(hor);
	float fy = static_cast<float>(ver);
	NormalizeDiagonalMovement(fx, fy);

	const bool moving = (fx != 0.f || fy != 0.f);
	moveComp.SetIsMoving(moving);
	moveComp.SetInputDirection(VelToDir(fx, fy));
	moveComp.SetMovementDirection({ fx, fy });

	// Handle dash input
	bool isDashing = dashComp && dashComp->IsDashing();

	if (dashComp && Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !isDashing && moving) {
		dashComp->StartDash();
	}

	// Update max speed
	physics.SetMaxSpeed(isDashing ? dashComp->dashMaxSpeed : MAX_SPEED);

	// Clear forces every frame
	physics.ClearForces();

	// Apply forces based on state
	if (moving) {
		if (isDashing) {
			physics.AddForce({ fx, fy }, dashComp->dashForce, 0.f);
		} else {
			// Normal movement
			physics.AddForce({ fx, fy }, MOVE_FORCE, 0.f);
		}
	}

	//if (!m_registry.Has_Component<InputComponent>(eID)) {
	//	return; // No input component, skip movement
	//}
	//auto& input = m_registry.Get_Component<InputComponent>(eID);

	//// Handle movement based on control scheme
	//switch (input.scheme) {
	//case CS_WASD:
	//	//if (Input::IsKeyHeld(GLFW_KEY_W)) transform.pos.y += speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_S)) transform.pos.y -= speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_A)) transform.pos.x -= speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_D)) transform.pos.x += speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_D)) transform.pos.x += speed * dt; 
	//    if (Input::IsKeyHeld(GLFW_KEY_W)) physics.AddForce({ 0,1 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_S)) physics.AddForce({ 0,-1 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_A)) physics.AddForce({ -1,0 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_D)) physics.AddForce({ 1,0 }, 1.f, 0.f);
	//	break;

	//case CS_IJKL:
	//	//if (Input::IsKeyHeld(GLFW_KEY_I)) transform.pos.y += speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_K)) transform.pos.y -= speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_J)) transform.pos.x -= speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_L)) transform.pos.x += speed * dt;
	//	if (Input::IsKeyHeld(GLFW_KEY_I)) physics.AddForce({ 0,1 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_K)) physics.AddForce({ 0,-1 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_J)) physics.AddForce({ -1,0 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_L)) physics.AddForce({ 1,0 }, 1.f, 0.f);
	//	if (m_registry.Has_Component<Animator::AnimatorComponent>(eID))
	//	{
	//		if ((Input::IsKeyHeld(GLFW_KEY_I) || Input::IsKeyHeld(GLFW_KEY_J) || Input::IsKeyHeld(GLFW_KEY_K) || Input::IsKeyHeld(GLFW_KEY_L)))
	//		{
	//			m_registry.Get_Component<Animator::AnimatorComponent>(eID).SetState(PS_WALK);
	//		}
	//		else
	//		{
	//			m_registry.Get_Component<Animator::AnimatorComponent>(eID).SetState(PS_IDLE);
	//		}

	//	}
	//	break;
	//case CS_ARROWS:
	//	//if (Input::IsKeyHeld(GLFW_KEY_UP)) transform.pos.y += speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_DOWN)) transform.pos.y -= speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_LEFT)) transform.pos.x -= speed * dt;
	//	//if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) transform.pos.x += speed * dt;
	//	if (Input::IsKeyHeld(GLFW_KEY_UP)) physics.AddForce({ 0,1 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_DOWN)) physics.AddForce({ 0,-1 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_LEFT)) physics.AddForce({ -1,0 }, 1.f, 0.f);
	//	if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) physics.AddForce({ 1,0 }, 1.f, 0.f);
	//	break;
	//}

	//// knockback test
	//int vert{0};
	//int hor{0};
	//if (Input::IsKeyHeld(GLFW_KEY_W)) vert += 1;
	//if (Input::IsKeyHeld(GLFW_KEY_S)) vert -= 1;
	//if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
	//if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;

	//Vec2 dir{ (float)hor, (float)vert };

	//if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) physics.AddForce(dir, dash);

}

namespace PlayerMove {
	inline void Start(Entity eID)
	{
		std::cout << "Player script started" << std::endl;
	}

	inline void Update(Entity eID, float dt) {
		if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID)) return;
		auto& moveComp = Registry::Get().Get_Component<Movement::MovementComponent>(eID);

		int hor = 0;
		int ver = 0;
		if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
		float fx = static_cast<float>(hor);
		float fy = static_cast<float>(ver);
		NormalizeDiagonalMovement(fx, fy);

		const bool moving = (fx != 0.f || fy != 0.f);
		moveComp.SetIsMoving(moving);
		moveComp.SetInputDirection(VelToDir(fx, fy));
		moveComp.SetMovementDirection({ fx, fy });


	}
}

void PlayerSystem::Initialize()
{
	{
		BehaviourFCT player;
		player.Init = [](Entity e) { PlayerMove::Start(e); };
		player.Update = [](Entity e, float dt) { PlayerMove::Update(e, dt); };
		player.End = [](Entity e) { PlayerMove::End(e); };
		LogicSystem::Get().RegisterScript("PlayerMovement", player, ScriptID::PLAYER);
	}
}

void PlayerSystem::Update(float dt)
{
	// check if game paused
	if (GSM_IsGamePaused()) {
		return;  // Don't update player movement or footsteps when paused
	}

	for (auto& [entity, comp] : m_registry.Get_ComponentMap<TagComponent>())
	{
		if (!m_registry.Has_Component<TransformComponent>(entity))
			continue;
		if (comp.tag == T_PLAYER)
		{
			// THIS IS A PLAYER, DO PLAYER THINGS
			WASD_Movement(entity, dt);
			UpdateFootsteps(entity, dt);
		}

	}
}
// ============================================================================
// FOOTSTEP IMPLEMENTATION
// ============================================================================

void PlayerSystem::UpdateFootsteps(Entity eID, float dt) {
	// Check if player is moving by checking input
	bool isMoving = false;

	if (m_registry.Has_Component<InputComponent>(eID)) {
		auto& input = m_registry.Get_Component<InputComponent>(eID);

		// Check movement keys based on control scheme
		switch (input.scheme) {
		case CS_WASD:
			isMoving = Input::IsKeyHeld(GLFW_KEY_W) ||
				Input::IsKeyHeld(GLFW_KEY_A) ||
				Input::IsKeyHeld(GLFW_KEY_S) ||
				Input::IsKeyHeld(GLFW_KEY_D);
			break;

		case CS_IJKL:
			isMoving = Input::IsKeyHeld(GLFW_KEY_I) ||
				Input::IsKeyHeld(GLFW_KEY_J) ||
				Input::IsKeyHeld(GLFW_KEY_K) ||
				Input::IsKeyHeld(GLFW_KEY_L);
			break;

		case CS_ARROWS:
			isMoving = Input::IsKeyHeld(GLFW_KEY_UP) ||
				Input::IsKeyHeld(GLFW_KEY_LEFT) ||
				Input::IsKeyHeld(GLFW_KEY_DOWN) ||
				Input::IsKeyHeld(GLFW_KEY_RIGHT);
			break;
		}
	}
	else {
		return;
	}

	// DEBUG: Print movement state once per second
	static float debugTimer = 0.0f;
	debugTimer += dt;
	if (debugTimer >= 1.0f) {
		debugTimer = 0.0f;
	}

	// Reset timer when not moving
	if (!isMoving) {
		footstepTimers[eID] = 0.0f;
		return;
	}

	// Increment timer
	footstepTimers[eID] += dt;

	// Play footstep when timer exceeds interval
	if (footstepTimers[eID] >= footstepInterval) {
		PlayFootstep(eID);
		footstepTimers[eID] = 0.0f;  // Reset timer
	}
}

void PlayerSystem::PlayFootstep(Entity eID) {
	// Get player position from transform
	if (!m_registry.Has_Component<TransformComponent>(eID)) {
		return;
	}

	auto& transform = m_registry.Get_Component<TransformComponent>(eID);

	// Play footstep sound at player position using spatial audio
	AudioSystem::PlaySoundAt(
		"player_footstep_wood",
		transform.pos.x,
		transform.pos.y,
		1.0f
	);
}