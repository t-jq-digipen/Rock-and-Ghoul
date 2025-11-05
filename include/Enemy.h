#pragma once
#include "Entity.h"

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
		EnemyAttackMode attackMode;
		EnemyState state;

		// Detection
		float detectionRadius;
		float loseInterestDistance;

		// Movement
		float moveSpeed;
		float chaseSpeed;
		float roamRadius;
		float idleDuration;
		float roamWaitDuration;
		bool canMove;

		// Melee
		float meleeCooldown;
		float meleeRange;
		bool canHit;

		// Projectile
		float shootCooldown;
		float reloadTime;
		int magazineSize;
		float shootRange;
		float engageRange;

		// Dodge Settings
		float dodgeDetectionRadius;
		float dodgeChancePerSecond;
		float dodgeForce;
		float dodgeDuration;

		// Avoidance Settings
		float enemyAvoidanceRadius;
		float avoidanceStrength;

		// References
		Entity enemy { 0 };
		Entity target{ 0 };
		Vec2 startPos{ 0.f, 0.f };

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

