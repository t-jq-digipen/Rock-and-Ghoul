#pragma once
#include "Entity.h"
#include <DataDefs.h>
#include <string>

namespace Enemy {

	// Set enemy to attack melee or projectile
	enum EnemyAttackMode : unsigned int
	{
		MELEE_ONLY,
		PROJECTILE_ONLY
	};

	// Set enemy AI behavior
	enum EnemyState : unsigned int
	{
		ENEMY_IDLE,
		ENEMY_ROAMING,
		ENEMY_DETECTING,
		ENEMY_CHASING,
		ENEMY_ATTACKING,
		ENEMY_COOLDOWN,
		ENEMY_STAGGERED
	};

	struct TransformQuery
	{
		virtual ~TransformQuery() = default;
		virtual Vec2 GetPosition(Entity e) const = 0;
	};

	class EnemyAIComponent
	{
	public:
		// General Settings
		EnemyAttackMode attackMode{ MELEE_ONLY };
		EnemyState state{ ENEMY_ROAMING };

		// Detection
		float detectionRadius{ 300.f };
		float loseInterestDistance{ 360.f };

		// Movement
		float moveSpeed{ 200.f };
		float chaseSpeed{ 300.f };
		float roamRadius{ 120.f };
		float idleDuration{ 0.8f };
		float roamWaitDuration{ 0.0f };
		bool canMove{ true };

		// Melee
		float meleeCooldown{ 0.8f };
		float meleeRange{ 60.f };
		bool canHit{ true };

		// Projectile
		float shootCooldown{ 1.2f };
		float reloadTime{ 1.5f };
		int magazineSize{ 3 };
		float shootRange{ 240.f };
		float engageRange{ 200.f };

		// Dodge Settings
		float dodgeDetectionRadius{ 0.f };
		float dodgeChancePerSecond{ 0.f };
		float dodgeForce{ 0.f };
		float dodgeDuration{ 0.f };

		// Avoidance Settings
		float enemyAvoidanceRadius{ 0.f };
		float avoidanceStrength{ 0.f };

		// References
		Entity enemy{ 0 };
		Entity target{ 0 };

		// Start position
		Vec2 startPos{ 0.f, 0.f };
		bool startPosInitialized{ false };

		// Runtime
		int currentAmmo{ 3 };
		bool isReloading{ false };
		float shootTimer{ 0.f };
		float reloadTimer{ 0.f };
		float stateTimer{ 0.f };
		Vec2 roamTarget{ 0.f, 0.f };

		// Outputs
		Vec2 desiredVelocity{ 0.f, 0.f };
		bool wantsMelee{ false };
		bool wantsShoot{ false };
		Vec2 shootDir{ 0.f, 0.f };

		// JSON
		std::string Serialize() const;
		static EnemyAIComponent DeSerialize(Entity eID, const std::string& data);
		Editor::ComponentData EditorData();
	};
}