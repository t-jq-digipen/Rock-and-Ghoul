/* ========================================================
 * @file		Physics.h
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @co-author	Yasmin Tazi	 | yasmin.t@digipen.edu
 * @date		30 OCT 2025
 *
 * @brief	Physics component for ECS
 *
 * This file declares physics related components and utilities
 * such as managing forces, calculating net velocities and
 * updating an entity based on the forces applied.
 *
 * The bulk of the updates happen in PhysicsSystem.cpp
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Remath.h"
#include <vector>
#include "Collision.h"
#include <string>
#include "DataDefs.h"
#include <nlohmann/json.hpp>

namespace Physics {

	/**
	 * @brief Represents a single force applied to an entity.
	 *
	 * A force has a direction, magnitude, and optional lifetime.
	 * - If lifetime == 0, the force is treated as an impulse (applies once, disappears).
	 * - If lifetime > 0, the force is continuous and applied until its age exceeds its lifetime.
	 */
	class ForceComp {
		Vec2 dir;				// Unit vector of direction
		float mag;				// Magnitude of vector
		Vec2 force;				// Cached final calculation of vector
		float age;				// Age of force
		float lifetime;			// Lifetime of force (if 0, means it just applies instantly for one frame then disappears)
		bool forceDirty;		// Flag to track if force needs recalculation

		void RecalculateForce();
	public:
		ForceComp();
		ForceComp(Vec2 direction, float magnitude, float duration = 0);
		~ForceComp() = default;

		void Update(float dt);
		void Set_Dir(Vec2 direction);
		void Set_Magnitude(float magnitude);

		bool Expired() const;
		bool IsImpulse() const;
		const Vec2& GetForce() const;
	};

	/**
	 * @brief Physics component for an entity.
	 *
	 * Stores velocity, hitboxes, and applied forces. Responsible for:
	 * - Updating velocity with damping and applied forces.
	 * - Managing collision state.
	 * - Synchronizing hitboxes with entity position.
	 *
	 * ForceList is now PhysicsComponent
	 */
	class PhysicsComponent {
		// Movement
		Vec2 vel;
		Vec2 acceleration;
		float decay;
		float maxSpeed;

		// Collision properties
		bool isStatic;
		bool willCollide;
		AABB aabbhitbox;
		Circle circlehitbox;

		// Force management (taken from ForceList)
		std::vector<ForceComp> forces;

		// Physical properties
		float mass;
		float restitution;
		float friction;

		// Collision tracking
		bool onGround;
		int collisionFrameCount; // prevents jitter effect

		void UpdateForces(float dt);
		void ApplyDamping(float dt);
		void ClampVelocity();

	public:
		PhysicsComponent();
		PhysicsComponent(float decayval, bool solid, AABB& aabbref, Circle& circleref, bool makeStatic = false);
		PhysicsComponent(const PhysicsComponent& other) = default;

		// Force management
		void AddForce(Vec2 const& direction, float magnitude, float duration = 0);
		void AddImpulse(Vec2 const& impulse); // Direct velocity change
		void ClearForces();
		bool HasForces() const;

		// Update methods
		void Update(float dt);
		void UpdateHitboxes(Vec2 const& pos);
		void IntegrateVelocity(Vec2& position, float dt); // Proper time-based integration

		// Collision setters/getters
		void Set_Collision(bool set);
		const bool& CollisionActive() const;

		// Hitbox setters/getters
		void Set_AABB(AABB const& newAABB);
		void Set_Circle(Circle const& newCircle);
		const AABB& Get_AABB() const;
		const Circle& Get_Circle() const;

		// Velocity control
		const Vec2& Get_Vel() const;
		void Set_Vel(Vec2 const& newVel);
		void StopX();
		void StopY();
		void Stop(); // stop all movement

		// Static/dynamic control
		bool IsStatic() const;
		void SetStatic(bool s);

		// Physical properties
		float GetMass() const;
		float GetRestitution() const;
		float GetFriction() const;
		float GetMaxSpeed() const;
		void SetMass(float m);
		void SetRestitution(float e);
		void SetFriction(float f);
		void SetMaxSpeed(float maxSpd);

		// Impulse physics
		void ApplyLinearImpulse(const Vec2& impulse);
		void ApplyFriction(float dt);

		// Collision state (IDK IF NEED)
		bool IsOnGround() const;
		void SetOnGround(bool grounded);
		void ResetCollisionFrame();
		bool ShouldSkipCollision() const;

		// Serialization
		std::string Serialize() const;
		static PhysicsComponent DeSerialize(Entity eID, const std::string& data);

		// Editor
		Editor::ComponentData EditorData();
	};
}