/* ========================================================
 * @file	    PlayerSystem.cpp
 * @author	    Tan Jun Quan | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date	    29 SEP 2025
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


constexpr float dash = 10.f;
constexpr float maxspeed = 10.f;
constexpr float speed = 500.f;

void PlayerSystem::WASD_Movement(Entity eID, float& dt)
{
	(void)dt;
	auto& physics = m_registry.Get_Component<Physics::PhysicsComponent>(eID);

	if (!m_registry.Has_Component<InputComponent>(eID)) {
		return; // No input component, skip movement
	}
	auto& input = m_registry.Get_Component<InputComponent>(eID);

	// Handle movement based on control scheme
	switch (input.scheme) {
	case CS_WASD:
		//if (Input::IsKeyHeld(GLFW_KEY_W)) transform.pos.y += speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_S)) transform.pos.y -= speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_A)) transform.pos.x -= speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_D)) transform.pos.x += speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_D)) transform.pos.x += speed * dt; 
		if (Input::IsKeyHeld(GLFW_KEY_W)) physics.AddForce({ 0,1 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_S)) physics.AddForce({ 0,-1 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_A)) physics.AddForce({ -1,0 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_D)) physics.AddForce({ 1,0 }, 1.f, 0.f);
		break;

	case CS_IJKL:
		//if (Input::IsKeyHeld(GLFW_KEY_I)) transform.pos.y += speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_K)) transform.pos.y -= speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_J)) transform.pos.x -= speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_L)) transform.pos.x += speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_I)) physics.AddForce({ 0,1 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_K)) physics.AddForce({ 0,-1 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_J)) physics.AddForce({ -1,0 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_L)) physics.AddForce({ 1,0 }, 1.f, 0.f);
		if (m_registry.Has_Component<Animator::AnimatorComponent>(eID))
		{
			if ((Input::IsKeyHeld(GLFW_KEY_I) || Input::IsKeyHeld(GLFW_KEY_J) || Input::IsKeyHeld(GLFW_KEY_K) || Input::IsKeyHeld(GLFW_KEY_L)))
			{
				m_registry.Get_Component<Animator::AnimatorComponent>(eID).SetState(PS_WALK);
			}
			else
			{
				m_registry.Get_Component<Animator::AnimatorComponent>(eID).SetState(PS_IDLE);
			}

		}
		break;
	case CS_ARROWS:
		//if (Input::IsKeyHeld(GLFW_KEY_UP)) transform.pos.y += speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_DOWN)) transform.pos.y -= speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_LEFT)) transform.pos.x -= speed * dt;
		//if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) transform.pos.x += speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_UP)) physics.AddForce({ 0,1 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_DOWN)) physics.AddForce({ 0,-1 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_LEFT)) physics.AddForce({ -1,0 }, 1.f, 0.f);
		if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) physics.AddForce({ 1,0 }, 1.f, 0.f);
		break;
	}

	// knockback test
	int vert{ 0 };
	int hor{ 0 };
	if (Input::IsKeyHeld(GLFW_KEY_W)) vert += 1;
	if (Input::IsKeyHeld(GLFW_KEY_S)) vert -= 1;
	if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
	if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;

	Vec2 dir{ (float)hor, (float)vert };

	if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) physics.AddForce(dir, dash);

	/*if (Input::IsKeyHeld(GLFW_KEY_W) && vel.y < maxspeed) physics.AddForce({ 0.f,1.f }, accel);
	if (Input::IsKeyHeld(GLFW_KEY_S) && vel.y > -maxspeed) physics.AddForce({ 0.f,-1.f }, accel);
	if (Input::IsKeyHeld(GLFW_KEY_A) && vel.x > -maxspeed) physics.AddForce({ -1.f,0.f }, accel);
	if (Input::IsKeyHeld(GLFW_KEY_D) && vel.x < maxspeed) physics.AddForce({ 1.f,0.f }, accel);*/


}

void PlayerSystem::Initialize()
{

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