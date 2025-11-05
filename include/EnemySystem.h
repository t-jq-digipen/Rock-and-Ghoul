#pragma once
#include "System.h"

// Set enemy AI behavior
enum EnemyState : unsigned int
{
	E_IDLE,
	E_ROAMING,
	E_DETECTING,
	E_CHASING,
	E_ATTACKING,
	E_COOLDOWN,
	E_STAGGERED
};

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
};
