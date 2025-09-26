#pragma once
#include "System.h"
#include "Collision.h"

class CollisionSystem : public ISystem {
public:
	CollisionSystem() {}
	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;
};