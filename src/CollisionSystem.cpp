#include "CollisionSystem.h"
#include "Registry.h"
#include "Component.h"
#include "Collision.h"

struct CollisionEvent {
	Entity entity1;
	Entity entity2;
	ComponentTypeID colliderType1;
	ComponentTypeID colliderType2;
	ComponentTypeID tag1;
	ComponentTypeID tag2;
};


void Update_AABB(AABBComponent& aabbcomp, float x, float y) 
{
	aabbcomp.aabb.max = { x + aabbcomp.width / 2.f,y + aabbcomp.height / 2.f };
	aabbcomp.aabb.min = { x - aabbcomp.width / 2.f,y - aabbcomp.height / 2.f };


}

void Update_Circle(CircleColliderComponent& circleComp, float x, float y)
{
	circleComp.active = true;
}

void CollisionSystem::Initialize()
{
	std::cout << "CollisionSystem Initialized!\n";
}

void CollisionSystem::Update(float dt)
{
	for (auto& [entity, aabb] : CM_AABB().getComponents())
	{
		//get x n y pos from transform comp
		TransformComponent transform = Get_TransformComponent(entity);
		Update_AABB(aabb, transform.pos.x, transform.pos.y);
	}

	// loop through aabbs/circles that need colliding(check if collili component.bool == true)
	/*
	* 
	* check everything collide first, then return events into a list(e.g player to enemy coll, enemy to projectile coll
	* then from there, update the physics for each "event" created
	*/
	//collect all entities into a vector
	auto& aabbEntities = CM_AABB().getComponents();
	std::vector<Entity> entityList;
	for (auto& [entity, aabb] : aabbEntities) {
		if (aabb.active) {
			entityList.push_back(entity);
		}
	}
	//check collisions and types
	for (int i = 0; i < entityList.size(); i++) {
		for (int j = i + 1; j < entityList.size(); j++) {
			Entity entity1 = entityList[i];
			Entity entity2 = entityList[j];

			AABBComponent& aabb1 = Get_AABBComponent(entity1);
			AABBComponent& aabb2 = Get_AABBComponent(entity2);

			if (CollisionIntersection_RectRect_Static(aabb1.aabb, aabb2.aabb)) {
				if (Has_Component(entity1, CT_Tag) && Has_Component(entity2, CT_Tag)) {
					TagComponent& tag1 = Get_TagComponent(entity1);
					TagComponent& tag2 = Get_TagComponent(entity2);
					CollisionType(entity1, entity2, tag1.tag, tag2.tag);
				}
			}
		}
	}
}

void CollisionSystem::Shutdown() {
	std::cout << "Collilisystem Shutdown!\n";
}
//helper function so not as much if else
void CollisionSystem::CollisionType(Entity entity1, Entity entity2, C_TagID tag1, C_TagID tag2) {
	//order independence w std::pair
	std::pair<C_TagID, C_TagID> collisionPair = {
		std::min(tag1, tag2), std::max(tag1, tag2)
	};

	if (collisionPair == std::make_pair(T_PLAYER, T_ENEMY)) {
		std::cout << "PLAYER vs ENEMY collision!" << std::endl;
		Entity player = (tag1 == T_PLAYER) ? entity1 : entity2;
		//maybe check rock entity 1 or 2
		//Entity rock = (tag1.tag == T_PLAYER) ? entity1 : entity2;

		// damage
		//if (Has_Component(player, CT_Health)) {
		//	HealthComponent& health = Get_HealthComponent(player);
		//	health.currhp -= 10.f;  // Take 10 damage
		//}
	}
	else if (collisionPair == std::make_pair(T_ENEMY, T_PROJECTILE)) {
		std::cout << "ENEMY vs PROJECTILE collision!" << std::endl;
	}
	else if (collisionPair == std::make_pair(T_PLAYER, T_PROJECTILE)) {
		std::cout << "PLAYER vs PROJECTILE collision!" << std::endl;
	}
}