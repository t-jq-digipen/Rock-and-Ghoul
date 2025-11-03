#pragma once
#include "System.h"
#include "Enemy.h"

class EnemySystem : public ISystem
{
public:
	EnemySystem() {}
	void Initialize() override;
	void Update(float dt) override;
	void Shutdown() override;

	void SetEnemyTransformQuery(const Enemy::TransformQuery* t) { tfq = t; }
private:
	const Enemy::TransformQuery* tfq{ nullptr };

	// helpers
	void ChangeState(Enemy::EnemyAIComponent& ai, Enemy::EnemyState next);
	bool DetectTarget(const Enemy::EnemyAIComponent& ai, const Vec2& enemyPos, const Vec2& targetPos) const;
	void PickNewRoamTarget(Enemy::EnemyAIComponent& ai);
};
