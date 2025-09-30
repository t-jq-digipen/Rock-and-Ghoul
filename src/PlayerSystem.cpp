#include "PlayerSystem.h"



constexpr float dash = 10.f;
constexpr float maxspeed = 10.f;
constexpr float speed = 500.f;

void WASD_Movement(Entity eID, float& dt)
{
	// very crude brake this shit gotta change
	/*if (!(Input::IsKeyHeld(GLFW_KEY_W) || Input::IsKeyHeld(GLFW_KEY_S)))
	{
		if (physics.vel.y >= 20.f)
		{
			physics.vel.y -= accel * dt;
		}
		else if (physics.vel.y <= -20.f)
		{
			physics.vel.y += accel * dt;
		}
		else
		{
			physics.vel.y = 0.f;
		}
	}
	if (!(Input::IsKeyHeld(GLFW_KEY_A) || Input::IsKeyHeld(GLFW_KEY_D)))
	{
		if (physics.vel.x >= 20.f)
		{
			physics.vel.x -= accel * dt;
		}
		else if (physics.vel.x <= -20.f)
		{
			physics.vel.x += accel * dt;
		}
		else
		{
			physics.vel.x = 0.f;
		}
	}*/


	/*if (Input::IsKeyHeld(GLFW_KEY_W)) physics.vel.y += accel * dt;
	if (Input::IsKeyHeld(GLFW_KEY_S)) physics.vel.y -= accel * dt;
	if (Input::IsKeyHeld(GLFW_KEY_A)) physics.vel.x -= accel * dt;
	if (Input::IsKeyHeld(GLFW_KEY_D)) physics.vel.x += accel * dt;

	if (physics.vel.x > 300.f) { physics.vel.x = 300.f; }
	if (physics.vel.x < -300.f) { physics.vel.x = -300.f; }
	if (physics.vel.y > 300.f) { physics.vel.y = 300.f; }
	if (physics.vel.y < -300.f) { physics.vel.y = -300.f; }*/

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
		if (Input::IsKeyHeld(GLFW_KEY_W)) transform.pos.y += speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_S)) transform.pos.y -= speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_A)) transform.pos.x -= speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_D)) transform.pos.x += speed * dt;
		break;

	case CS_IJKL:
		if (Input::IsKeyHeld(GLFW_KEY_I)) transform.pos.y += speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_K)) transform.pos.y -= speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_J)) transform.pos.x -= speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_L)) transform.pos.x += speed * dt;
		break;
	case CS_ARROWS:
		if (Input::IsKeyHeld(GLFW_KEY_UP)) transform.pos.y += speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_DOWN)) transform.pos.y -= speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_LEFT)) transform.pos.x -= speed * dt;
		if (Input::IsKeyHeld(GLFW_KEY_RIGHT)) transform.pos.x += speed * dt;
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
		if (Has_Component(entity, CT_Sprite)) {
			auto& sprite = Get_SpriteComponent(entity);

			if (Input::IsKeyHeld(GLFW_KEY_Q)) sprite.size.x += 100.f * dt;
			if (Input::IsKeyHeld(GLFW_KEY_E)) sprite.size.x = std::max(10.f, sprite.size.x - 100.f * dt);

			if (Input::IsKeyHeld(GLFW_KEY_T)) sprite.size.y += 100.f * dt;
			if (Input::IsKeyHeld(GLFW_KEY_F)) sprite.size.y = std::max(10.f, sprite.size.y - 100.f * dt);

			if (Input::IsKeyHeld(GLFW_KEY_Z)) trans.rot -= 90.f * dt; // counter-clockwise
			if (Input::IsKeyHeld(GLFW_KEY_X)) trans.rot += 90.f * dt; // clockwise
		}
	}
}
