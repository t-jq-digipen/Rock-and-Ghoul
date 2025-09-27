#include "PlayerSystem.h"



constexpr float accel = .1f;
constexpr float maxspeed = 10.f;

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

	if (Input::IsKeyHeld(GLFW_KEY_W) && vel.y < maxspeed) physics.AddForce({ 0.f,1.f }, accel);
	if (Input::IsKeyHeld(GLFW_KEY_S) && vel.y > -maxspeed) physics.AddForce({ 0.f,-1.f }, accel);
	if (Input::IsKeyHeld(GLFW_KEY_A) && vel.x > -maxspeed) physics.AddForce({ -1.f,0.f }, accel);
	if (Input::IsKeyHeld(GLFW_KEY_D) && vel.x < maxspeed) physics.AddForce({ 1.f,0.f }, accel);

	


}

void PlayerSystem::Initialize()
{

}

void PlayerSystem::Update(float dt)
{
	for (auto& [entity, comp] : CM_Tag().getComponents())
	{
		if (comp.tag == T_PLAYER)
		{
			// THIS IS A PLAYER, DO PLAYER THINGS!

			WASD_Movement(entity, dt);

		}
	}
}
