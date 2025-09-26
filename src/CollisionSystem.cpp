#include "CollisionSystem.h"
#include "Registry.h"
#include "Component.h"


void Update_AABB(AABBComponent& aabbcomp, float x, float y) 
{
	aabbcomp.aabb.max = { x + aabbcomp.width / 2.f,y + aabbcomp.height / 2.f };
	aabbcomp.aabb.min = { x - aabbcomp.width / 2.f,y - aabbcomp.height / 2.f };
}


void CollisionSystem::Initialize()
{

}

void CollisionSystem::Update(float dt)
{
	for (auto& [entity, aabb] : CM_AABB().getComponents())
	{
		//get x n y pos from transform comp
		TransformComponent temp = Get_TransformComponent(entity);
		Update_AABB(aabb, temp.pos.x, temp.pos.y);
	}

	// loop through aabbs/circles that need colliding(check if collili component.bool == true)
	/*
	* check everything collide first, then return events into a list(e.g player to enemy coll, enemy to projectile coll
	* then from there, update the physics for each "event" created
	*/

}

void CollisionSystem::Shutdown() {
	std::cout << "Collilisystem Shutdown!\n";
}