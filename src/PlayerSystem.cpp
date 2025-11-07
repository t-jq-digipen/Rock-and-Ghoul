/* ========================================================
 * @file	    PlayerSystem.cpp
 * @author	    Tan Jun Quan	   | t.junquan@digipen.edu
 * @co-author   Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @co-author   Yasmin Tazi		   | yasmin.t@digipen.edu
 * @co-author   Manfred Sim | manfred.sim@digipen.edu
 * @date	    06 NOV 2025
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

 // CONSTANTS for responsive, non-tunneling movement, can adjust
constexpr float MOVE_FORCE = 1000.f;    // Acceleration force
constexpr float MAX_SPEED = 200.f;      // Top speed cap
constexpr float DASH_IMPULSE = 1000.f;   // Dash boost

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
	(void)dt;

	if (!m_registry.Has_Component<Movement::MovementComponent>(eID)) return;
	auto& moveComp = m_registry.Get_Component<Movement::MovementComponent>(eID);
	auto& physics = m_registry.Get_Component<Physics::PhysicsComponent>(eID);

	// To be reworked when InputComponent no longer needed???

	if (!m_registry.Has_Component<InputComponent>(eID)) {
		moveComp.SetIsMoving(false);
		moveComp.SetInputDirection(Movement::MOVE_NONE);
		moveComp.SetMovementDirection({ 0.f, 0.f });
		return;
	}

	// Ensure max speed is set (prevents tunneling)
	if (physics.GetMaxSpeed() < MAX_SPEED) {
		physics.SetMaxSpeed(MAX_SPEED);
	}
	// Clear forces every frame so that player instantly switches directions
	physics.ClearForces();

	const auto& input = m_registry.Get_Component<InputComponent>(eID);

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

	float fx = static_cast<float>(hor);
	float fy = static_cast<float>(ver);
	NormalizeDiagonalMovement(fx, fy);

	const bool moving = (fx != 0.f || fy != 0.f);
	moveComp.SetIsMoving(moving);
	moveComp.SetInputDirection(VelToDir(fx, fy));
	moveComp.SetMovementDirection({ fx, fy });

	// Clear forces every frame
	physics.ClearForces();

	// Dash
	constexpr float DASH_IMPULSE = 700.f;

	if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && moving) {
		Vec2 dashImpulse = { fx * DASH_IMPULSE, fy * DASH_IMPULSE };
		physics.AddImpulse(dashImpulse);
	}

	// Adjust max speed based on current speed
	float currentSpeed = RG_Length(physics.Get_Vel());
	if (currentSpeed > 500.f) {
		physics.SetMaxSpeed(1000.f);  // For da dash
	}
	else {
		physics.SetMaxSpeed(500.f);   // Normal speed
	}

	if (moving) {
		physics.AddForce({ fx, fy }, MOVE_FORCE, 0.f);
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

	///*if (Input::IsKeyHeld(GLFW_KEY_W) && vel.y < maxspeed) physics.AddForce({ 0.f,1.f }, accel);
	//if (Input::IsKeyHeld(GLFW_KEY_S) && vel.y > -maxspeed) physics.AddForce({ 0.f,-1.f }, accel);
	//if (Input::IsKeyHeld(GLFW_KEY_A) && vel.x > -maxspeed) physics.AddForce({ -1.f,0.f }, accel);
	//if (Input::IsKeyHeld(GLFW_KEY_D) && vel.x < maxspeed) physics.AddForce({ 1.f,0.f }, accel);*/

	// DASH - Working impulse
	if (Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
		Vec2 dashDir{ (float)hor, (float)ver };

		fx = static_cast<float>(hor);
		fy = static_cast<float>(ver);
		NormalizeDiagonalMovement(fx, fy);

		if (RG_LengthSquared(dashDir) > 0.01f) {
			dashDir = RG_Normalize(dashDir);
			Vec2 dashImpulse = RG_Scale(dashDir, DASH_IMPULSE);
			physics.ApplyLinearImpulse(dashImpulse);
		}
	}
}

namespace PlayerMove {
	std::unordered_map<Entity, size_t> subscriberID;

	void Dash(Entity eID) {
		if (!Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) return;
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);

		// Vec2 dashDir = moveComp.GetMovementDirection();
		int hor = 0;
		int ver = 0;
		if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
		float fx = static_cast<float>(hor);
		float fy = static_cast<float>(ver);

		Vec2 dashDir{ fx, fy };

		NormalizeDiagonalMovement(dashDir.x, dashDir.y);

		if (RG_LengthSquared(dashDir) > 0.01f) {
			dashDir = RG_Normalize(dashDir);
			Vec2 dashImpulse = RG_Scale(dashDir, DASH_IMPULSE);
			physics.ApplyLinearImpulse(dashImpulse);
		}

	}

	inline void Start(Entity eID)
	{
		std::cout << "Player script started" << std::endl;

		subscriberID[eID] = EventBus::Get().Subscribe<KeyPressedEvent>([eID](const KeyPressedEvent& e) {

			if (e.key == GLFW_KEY_LEFT_SHIFT)
			{
				Dash(eID);
			}
			});
	}

	inline void Update(Entity eID, float dt) {
		(void)dt;
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


		if (!Registry::Get().Has_Component<Animator::AnimatorComponent>(eID)) return;
		auto& animComp = Registry::Get().Get_Component<Animator::AnimatorComponent>(eID);

		PlayerState animstate = (moving) ? PS_WALK : PS_IDLE;
		animComp.SetState(animstate);
	}

	inline void End(Entity e)
	{
		EventBus::Get().Unsubscribe<KeyPressedEvent>(subscriberID[e]);
		subscriberID.erase(e);
		std::cout << "Player script ended" << std::endl;
	}
} // end namespace PlayerMove

namespace RockScript {
	static float stunnedtimer = 1.f;

	std::unordered_map<Entity, size_t> subscriberID;

	void Dash(Entity eID) {
		if (stunnedtimer < 1.f) { return; }
		if (!Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) return;
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);

		// Vec2 dashDir = moveComp.GetMovementDirection();
		int hor = 0;
		int ver = 0;
		if (Input::IsKeyHeld(GLFW_KEY_W)) ver += 1;
		if (Input::IsKeyHeld(GLFW_KEY_S)) ver -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_A)) hor -= 1;
		if (Input::IsKeyHeld(GLFW_KEY_D)) hor += 1;
		float fx = static_cast<float>(hor);
		float fy = static_cast<float>(ver);

		Vec2 dashDir{ fx, fy };

		NormalizeDiagonalMovement(dashDir.x, dashDir.y);

		if (RG_LengthSquared(dashDir) > 0.01f) {
			dashDir = RG_Normalize(dashDir);
			Vec2 dashImpulse = RG_Scale(dashDir, DASH_IMPULSE);
			physics.ApplyLinearImpulse(dashImpulse);
		}

	}

	inline void Start(Entity eID)
	{
		std::cout << "Rock script started" << std::endl;

		subscriberID[eID] = EventBus::Get().Subscribe<KeyPressedEvent>([eID](const KeyPressedEvent& e) {

			if (e.key == GLFW_KEY_LEFT_SHIFT)
			{
				Dash(eID);
			}
			});
	}

	inline void Update(Entity eID, float dt) {
		(void)dt;


		if (!Registry::Get().Has_Component<Movement::MovementComponent>(eID) || !Registry::Get().Has_Component<Physics::PhysicsComponent>(eID)) return;
		auto& moveComp = Registry::Get().Get_Component<Movement::MovementComponent>(eID);
		auto& physics = Registry::Get().Get_Component<Physics::PhysicsComponent>(eID);

		int animstate = 0; //idle

		// MOVEMENT
		// ================================================================================
		if (stunnedtimer >= 1.f)
		{
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

			if (moving) { animstate = 1; }
		}
		else
		{
			stunnedtimer += dt;
			animstate = 2;
		}
		if (Input::IsKeyPressed(GLFW_KEY_G))
		{
			stunnedtimer = 0.f;
			animstate = 2;
			physics.ApplyLinearImpulse(RG_Scale({ -1,0 }, DASH_IMPULSE));
		}

		if (!Registry::Get().Has_Component<Animator::AnimatorComponent>(eID)) return;
		auto& animComp = Registry::Get().Get_Component<Animator::AnimatorComponent>(eID);

		animComp.SetState(animstate);
	}

	inline void End(Entity e)
	{
		EventBus::Get().Unsubscribe<KeyPressedEvent>(subscriberID[e]);
		subscriberID.erase(e);
		std::cout << "Rock script ended" << std::endl;
	}
} // end namespace RockScript

void PlayerSystem::Initialize()
{
	{
		BehaviourFCT player;
		player.Init = [](Entity e) { PlayerMove::Start(e); };
		player.Update = [](Entity e, float dt) { PlayerMove::Update(e, dt); };
		player.End = [](Entity e) { PlayerMove::End(e); };
		LogicSystem::Get().RegisterScript("PlayerMovement", player, ScriptID::PLAYER);
	}
	{
		BehaviourFCT player;
		player.Init = [](Entity e) { RockScript::Start(e); };
		player.Update = [](Entity e, float dt) { RockScript::Update(e, dt); };
		player.End = [](Entity e) { RockScript::End(e); };
		LogicSystem::Get().RegisterScript("RockScript", player, ScriptID::ROCK_SCRIPT);
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