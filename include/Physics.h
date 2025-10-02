/* ========================================================
 * @file	Physics.h
 * @author	Tan Jun Quan
 * @date	29 SEP 2025
 *
 * @brief	Physics component for ECS
 *
 * This file declares physics related components and utilities
 * such as managing forces, calculating net velocities and
 * updating an entity based on the forces applied.
 * 
 * The bulk of the updates happen in PhysicsSystem.cpp
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */



#pragma once
#include "Remath.h"
#include <vector>
#include "Collision.h"


namespace Physics {


	// linear forces? drag? friction? i wna dies
	/**
	 * @brief Represents a single force applied to an entity.
	 *
	 * A force has a direction, magnitude, and optional lifetime.
	 * - If `lifetime == 0`, the force is treated as an **impulse** (applies once, disappears).
	 * - If `lifetime > 0`, the force is continuous and applied until its age exceeds its lifetime.
	 *
	 * The class computes a final force vector from direction and magnitude,
	 * automatically normalizing the direction vector.
	 */
	class ForceComp {
		Vec2 dir;				// Unit vector of direction
		float mag;				// Magnitude of vector
		Vec2 force;				// Final calculation of vector
		float age;				// Age of force
		float lifetime;			// Lifetime of force (if 0, means it just applies instantly for one frame then disappears)
	public:
		/**
		 * @brief Default constructor. Initializes a zero force.
		 */
		ForceComp();
		/**
		 * @brief Construct a force with a given direction, magnitude, and optional lifetime.
		 * @param direction Direction of the force (normalized internally).
		 * @param magnitude Magnitude of the force.
		 * @param duration Lifetime of the force in seconds (0 = impulse).
		 */
		ForceComp(Vec2 direction, float magnitude, float duration = 0);
		~ForceComp() = default;		// should be ok... if using normal types without assigned memory

		/**
		 * @brief Update the force state.
		 *
		 * - Recomputes the force vector (`dir * mag`).
		 * - Increments age if lifetime > 0.
		 */
		void Update();

		/**
		 * @brief Set the direction of the force.
		 * @param direction New direction vector (normalized internally).
		 */
		void Set_Dir(Vec2 direction);
		/**
		 * @brief Set the magnitude of the force.
		 * @param magnitude New magnitude value.
		 */
		void Set_Magnitude(float magnitude);
		/**
		 * @brief Check if this force has expired.
		 * @return True if `age >= lifetime` (and lifetime > 0), false otherwise.
		 */
		bool Expired();
		/**
		 * @brief Check if this force is an impulse.
		 * @return True if `lifetime == 0`.
		 */
		bool IsImpulse();
		/**
		 * @brief Get the current force vector.
		 * @return The force as a Vec2.
		 */
		Vec2 GetForce();
	};


	/**
	 * @brief A container for managing multiple forces applied to an entity.
	 *
	 * The ForceList tracks all active ForceComp instances, updates them each frame,
	 * and computes the net velocity contribution from all forces.
	 */
	class ForceList {
		std::vector<ForceComp> forces;		// List of forces
		Vec2 net_vel;						// Net velocity from forces

		/**
		 * @brief Compute velocity change from a given force.
		 *
		 * Impulses return their full vector immediately.
		 * Continuous forces scale with `g_dt`.
		 *
		 * @param force The force to evaluate.
		 * @return Velocity change contribution from this force.
		 */
		Vec2 Update_NetVel(ForceComp force);
	public:
		/**
		 * @brief Add a new force to the list.
		 * @param force The force to add.
		 */
		void Add_Force(ForceComp);
		/**
		 * @brief Update all forces and compute net velocity.
		 *
		 * - Updates each force (recomputes vectors, increments age).
		 * - Adds their contributions to `net_vel`.
		 * - Removes expired forces.
		 */
		void Update();
		/**
		 * @brief Check if the list contains any forces.
		 * @return True if at least one force exists.
		 */
		bool HasForces();
		/**
		 * @brief Get the net velocity change from all forces.
		 * @return Const reference to the net velocity vector.
		 */
		const Vec2& Get_NetVel() const;
		/**
		 * @brief Clear all forces.
		 */
		void Reset();
	};


	/**
	 * @brief Physics component for an entity.
	 *
	 * Stores velocity, hitboxes, and applied forces. Responsible for:
	 * - Updating velocity with damping and applied forces.
	 * - Managing collision state.
	 * - Synchronizing hitboxes with entity position.
	 */
	class PhysicsComponent {
		Vec2 vel;
		float decay;
		bool isStatic;
		bool willCollide;
		AABB aabbhitbox;
		Circle circlehitbox;
		ForceList forcelist;
		
		/**
		 * @brief Update forces and apply their contributions to velocity.
		 */
		void UpdateForces();
	public:
		/**
		 * @brief Default constructor. Initializes zero velocity and empty hitboxes.
		 */
		PhysicsComponent();
		PhysicsComponent(float decayval, bool solid, AABB& aabbref, Circle& circleref, bool makeStatic = false);

		PhysicsComponent(const PhysicsComponent& other) = default;

		void AddForce(Vec2 const& direction, float magnitude, float duration = 0);
		/**
		 * @brief Update this physics component.
		 *
		 * Applies damping and updates all forces.
		 */
		void Update();
		/**
		 * @brief Update hitbox positions based on entity position.
		 * @param pos New entity position.
		 */
		void UpdateHitboxes(Vec2 const& pos);
		/**
		 * @brief Enable or disable collision.
		 * @param set True to enable, false to disable.
		 */
		void Set_Collision(bool set);
		/**
		 * @brief Set the AABB hitbox.
		 * @param newAABB The new AABB.
		 */
		void Set_AABB(AABB const& newAABB);
		/**
		 * @brief Set the Circle hitbox.
		 * @param newCircle The new Circle.
		 */
		void Set_Circle(Circle const& newCircle);
		/**
		 * @brief Get the current velocity.
		 * @return Const reference to velocity vector.
		 */
		const Vec2& Get_Vel() const;
		/**
		 * @brief Get the AABB hitbox.
		 * @return Const reference to the AABB.
		 */
		const AABB& Get_AABB() const;
		/**
		 * @brief Get the Circle hitbox.
		 * @return Const reference to the Circle.
		 */
		const Circle& Get_Circle() const;
		/**
		 * @brief Check if collision is enabled.
		 * @return Const reference to collision flag.
		 */
		const bool& CollisionActive() const;
		/**
		 * @brief Apply damping to velocity.
		 *
		 * If `decay > 0`, velocity is scaled by `pow(decay, g_dt)`.
		 */
		void Damping();

		/**
		 * @brief Set the velocity directly.
		 * @param newVel New velocity vector.
		 */
		void Set_Vel(Vec2 const& newVel);
		/**
		 * @brief Stop horizontal movement (x velocity = 0).
		 */
		void StopX() { vel.x = 0.f; }
		/**
		 * @brief Stop vertical movement (y velocity = 0).
		 */
		void StopY() { vel.y = 0.f; }

		bool IsStatic() const { return isStatic; }
		void SetStatic(bool s) { isStatic = s; }
	};
}

// movement can be like while vel < speed, force set vel...?