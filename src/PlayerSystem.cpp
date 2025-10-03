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


constexpr float dash = 10.f;
constexpr float maxspeed = 10.f;
constexpr float speed = 500.f;

void WASD_Movement(Entity eID, float& dt)
{
	auto& physics = Get_PhysicsComponent(eID);
	const Vec2& vel = physics.Get_Vel();

	auto& transform = Get_TransformComponent(eID);

	if (!Has_Component(eID, CT_Input)) {
		return; // No input component, skip movement
	}
	auto& input = Get_InputComponent(eID);

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
		if (CM_Animator().hasComponent(eID))
		{
			if ((Input::IsKeyHeld(GLFW_KEY_I) || Input::IsKeyHeld(GLFW_KEY_J) || Input::IsKeyHeld(GLFW_KEY_K) || Input::IsKeyHeld(GLFW_KEY_L)))
			{
				CM_Animator().getComponent(eID).SetState(PS_WALK);
			}
			else
			{
				CM_Animator().getComponent(eID).SetState(PS_IDLE);
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
	int vert{0};
	int hor{0};
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
	for (auto& [entity, comp] : CM_Tag().getComponents())
	{
		if (!Has_Component(entity, CT_Transform))
			continue;

		auto& trans = Get_TransformComponent(entity);
		if (comp.tag == T_PLAYER)
		{
			// THIS IS A PLAYER, DO PLAYER THINGS
			trans.Prevpos = trans.pos;
			WASD_Movement(entity, dt);

		}
	}
}
