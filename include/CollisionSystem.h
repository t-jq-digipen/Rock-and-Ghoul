#pragma once
#include "System.h"
#include "Collision.h"
#include "Component.h"
struct CollisionEvent;
class CollisionSystem : public ISystem {
public:
	CollisionSystem() {}
	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;

private:
	void CollisionType(Entity entity1, Entity entity2, C_TagID tag1, C_TagID tag2);
};