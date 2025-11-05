#pragma once
#include "System.h"
#include "Enemy.h"

namespace EnemyAI {

	inline void Start(Entity eID);

	inline void Update(Entity eID, float dt);

	inline void End(Entity eID)
	{
		std::cout << "Enemy script ended" << std::endl;
	}
}

class EnemySystem : public ISystem
{
public:
	EnemySystem() {}
	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;

	void SetEnemyTransformQuery(const Enemy::TransformQuery* t) { tfq = t; }

	// helpers
	void ChangeState(Enemy::EnemyAIComponent& ai, Enemy::EnemyState next);
	void PickNewRoamTarget(Enemy::EnemyAIComponent& ai);
	Entity FindNearestPlayer(const Vec2& enemyPos, float& outDist) const;
	bool DetectTarget(const Enemy::EnemyAIComponent& ai, const Vec2& enemyPos, const Vec2& targetPos) const;

private:
	const Enemy::TransformQuery* tfq{ nullptr };
};
