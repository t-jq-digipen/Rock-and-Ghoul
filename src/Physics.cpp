/* ========================================================
 * @file		Physics.cpp
 * @author		Yasmin Tazi  | yasmin.t@digipen.edu
 * @co-author	Tan Jun Quan | t.junquan@digipen.edu
 * @date		19 NOV 2025
 *
 * @brief	Physics component for ECS
 *
 * This file implements physics related components and utilities
 * such as managing forces, calculating net velocities and
 * updating an entity based on the forces applied.
 *
 * Includes backward compatibility migration that converts old Physics
 * collision data to ColliderComponent (skipped if Collider already exists).
 *
 * The bulk of the updates happen in PhysicsSystem.cpp
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "Physics.h"
#include "CoreEngine.h"
#include "CollisionManager.h"
#include <cmath>

namespace Physics {

	// ================================================================================
	// FORCE COMPONENT
	// ================================================================================

	ForceComp::ForceComp()
		: dir{ 0.f, 0.f }
		, mag{ 0.f }
		, force{ 0.f, 0.f }
		, age{ 0.f }
		, lifetime{ 0.f }
		, forceDirty{ false }
	{
	}

	ForceComp::ForceComp(Vec2 direction, float magnitude, float duration)
		: dir{ RG_Normalize(direction) }
		, mag{ magnitude }
		, force{ 0.f, 0.f }
		, age{ 0.f }
		, lifetime{ duration }
		, forceDirty{ true }
	{
		RecalculateForce();
	}

	void ForceComp::RecalculateForce() {
		force = RG_Scale(dir, mag);
		forceDirty = false;
	}

	void ForceComp::Update(float dt) {
		// Only recalculate if force vector has changed
		if (forceDirty) {
			RecalculateForce();
		}

		// Update age for non-impulse forces
		if (lifetime > 0.f) {
			age += dt;
		}
	}

	const Vec2& ForceComp::GetForce() const {
		return force;
	}

	void ForceComp::Set_Dir(Vec2 direction) {
		dir = RG_Normalize(direction);
		forceDirty = true;
	}

	void ForceComp::Set_Magnitude(float magnitude) {
		mag = magnitude;
		forceDirty = true;
	}

	bool ForceComp::IsImpulse() const {
		return lifetime == 0.f;
	}

	bool ForceComp::Expired() const {
		return (lifetime > 0.f && age >= lifetime);
	}

	// ================================================================================
	// PHYSICS COMPONENT
	// ================================================================================

	PhysicsComponent::PhysicsComponent()
		: vel{ 0.f, 0.f }
		, acceleration{ 0.f, 0.f }
		, decay{ 0.015f }       // Slight damping
		, maxSpeed{ 500.f }    // Prevent tunneling
		, mass{ 1.f }
		, restitution{ 0.3f }  // Some bounce
		, friction{ 0.1f }     // Slight friction
		, isStatic{ false }
		, onGround{ false }
		, collisionFrameCount{ 0 }
	{
		forces.reserve(8);
	}

	PhysicsComponent::PhysicsComponent(float decayval, float maxSpd, bool makeStatic)
		: vel{ 0.f, 0.f }
		, acceleration{ 0.f, 0.f }
		, decay{ decayval }
		, maxSpeed{ maxSpd }
		, mass{ 1.f }
		, restitution{ 0.3f }
		, friction{ 0.1f }
		, isStatic{ makeStatic }
		, onGround{ false }
		, collisionFrameCount{ 0 }
	{
		forces.reserve(8);
	}
	// ================================================================================
	// FORCE MANAGEMENT
	// ================================================================================

	void PhysicsComponent::AddForce(Vec2 const& direction, float magnitude, float duration) {
		forces.emplace_back(direction, magnitude, duration);
	}

	void PhysicsComponent::AddImpulse(Vec2 const& impulse) {
		vel = RG_Add(vel, impulse);
		//ClampVelocity();
	}

	void PhysicsComponent::ClearForces() {
		forces.clear();
		acceleration = { 0.f, 0.f };
	}

	bool PhysicsComponent::HasForces() const {
		return !forces.empty();
	}

	void PhysicsComponent::UpdateForces(float dt) {
		if (forces.empty()) {
			return;
		}

		Vec2 netAcceleration{ 0.f, 0.f };

		// Process all forces and accumulate
		for (auto it = forces.begin(); it != forces.end(); ) {
			it->Update(dt);

			const Vec2& force = it->GetForce();

			if (it->IsImpulse()) {
				// Impulse: immediate velocity change (F * dt / m)
				Vec2 deltaV = RG_Scale(force, dt / mass);
				vel = RG_Add(vel, deltaV);
			}
			else {
				// Continuous force: acceleration (F / m)
				Vec2 accel = RG_Scale(force, 1.f / mass);
				netAcceleration = RG_Add(netAcceleration, accel);
			}

			// Remove expired forces
			if (it->Expired()) {
				it = forces.erase(it);
			}
			else {
				++it;
			}
		}

		// Apply continuous acceleration
		acceleration = netAcceleration;
	}

	// ================================================================================
	// VELOCITY MANAGEMENT
	// ================================================================================

	void PhysicsComponent::ClampVelocity() {
		float speedSq = RG_LengthSquared(vel);
		float maxSpeedSq = maxSpeed * maxSpeed;

		if (speedSq > maxSpeedSq) {
			float speed = RG_Sqrt(speedSq);
			vel = RG_Scale(vel, maxSpeed / speed);
		}
	}

	void PhysicsComponent::ApplyDamping(float dt) {
		// Exponential decay for smooth deceleration
		if (decay < 1.f && decay > 0.f) {
			float factor = std::pow(decay, dt);
			vel = RG_Scale(vel, factor);
		}

		// Stop if velocity is negligible (prevents endless tiny movements)
		if (RG_LengthSquared(vel) < 0.01f) {
			vel = { 0.f, 0.f };
		}
	}

	void PhysicsComponent::ApplyFriction(float dt) {
		if (friction > 0.f && onGround) {
			float frictionForce = friction * dt;
			float speed = RG_Length(vel);

			if (speed > 0.f) {
				float reduction = std::min(frictionForce, speed);
				vel = RG_Scale(vel, (speed - reduction) / speed);
			}
		}
	}

	// ================================================================================
	// UPDATE METHODS
	// ================================================================================

	void PhysicsComponent::Update(float dt) {
		if (isStatic) {
			return; // Static objects don't move
		}

		// Update collision frame counter
		if (collisionFrameCount > 0) {
			collisionFrameCount--;
		}

		// Apply forces and get acceleration
		UpdateForces(dt);

		// Integrate acceleration into velocity (time-based)
		vel = RG_Add(vel, RG_Scale(acceleration, dt));

		// Apply friction if on ground
		ApplyFriction(dt);

		// Apply damping (air resistance)
		ApplyDamping(dt);

		// Clamp to max speed (prevent tunneling)
		ClampVelocity();
	}

	void PhysicsComponent::IntegrateVelocity(Vec2& position, float dt) {
		if (isStatic) {
			return;
		}

		// Semi-implicit Euler integration
		// v(t+dt) = v(t) + a*dt  (already done in Update)
		// p(t+dt) = p(t) + v(t+dt)*dt
		position = RG_Add(position, RG_Scale(vel, dt));
	}

	// ================================================================================
	// VELOCITY CONTROL
	// ================================================================================

	const Vec2& PhysicsComponent::Get_Vel() const {
		return vel;
	}

	void PhysicsComponent::Set_Vel(Vec2 const& newVel) {
		vel = newVel;
	}

	void PhysicsComponent::StopX() {
		vel.x = 0.f;
	}

	void PhysicsComponent::StopY() {
		vel.y = 0.f;
	}
	
	void PhysicsComponent::Stop() {
		vel = { 0.f, 0.f };
		acceleration = { 0.f, 0.f };
		ClearForces();
	}


	// ================================================================================
	// STATIC/DYNAMIC CONTROL
	// ================================================================================

	bool PhysicsComponent::IsStatic() const {
		return isStatic;
	}

	void PhysicsComponent::SetStatic(bool s) {
		isStatic = s;
		if (s) {
			vel = { 0.f, 0.f };
			acceleration = { 0.f, 0.f };
			ClearForces();
		}
	}

	// ================================================================================
	// PHYSICAL PROPERTIES
	// ================================================================================

	float PhysicsComponent::GetMass() const {
		return mass;
	}

	float PhysicsComponent::GetRestitution() const {
		return restitution;
	}
	
	float PhysicsComponent::GetFriction() const {
		return friction;
	}

	float PhysicsComponent::GetMaxSpeed() const {
		return maxSpeed;
	}

	void PhysicsComponent::SetMass(float m) {
		mass = m;
	}

	void PhysicsComponent::SetRestitution(float e) {
		restitution = std::max(0.f, std::min(1.f, e));
	}

	void PhysicsComponent::SetFriction(float f) {
		friction = std::max(0.f, f);
	}

	void PhysicsComponent::SetMaxSpeed(float maxSpd) {
		maxSpeed = std::max(1.f, maxSpd);
	}

	// ================================================================================
	// IMPULSE PHYSICS
	// ================================================================================

	void PhysicsComponent::ApplyLinearImpulse(const Vec2& impulse) {
		if (!isStatic) {
			// J = m * v  =>  v = J / m
			Vec2 deltaV = RG_Scale(impulse, 1.0f / mass);
			vel = RG_Add(vel, deltaV);
			// ClampVelocity();

			// Mark that we just had a collision (to prevent jitter)
			collisionFrameCount = 2;
		}
	}

	// ================================================================================
	// COLLISION STATE
	// ================================================================================

	bool PhysicsComponent::IsOnGround() const {
		return onGround;
	}

	void PhysicsComponent::SetOnGround(bool grounded) {
		onGround = grounded;
	}

	void PhysicsComponent::ResetCollisionFrame() {
		collisionFrameCount = 0;
	}

	bool PhysicsComponent::ShouldSkipCollision() const {
		// Skip if we just resolved a collision (prevents jitter)
		return collisionFrameCount > 0;
	}


	// ================================================================================
	// SERIALIZATION
	// ================================================================================

	std::string PhysicsComponent::Serialize() const {
		nlohmann::json j = {
			{"velX", vel.x},
			{"velY", vel.y},
			{"decay", decay},
			{"maxSpeed", maxSpeed},
			{"isStatic", isStatic},
			{"mass", mass},
			{"restitution", restitution},
			{"friction", friction}
		};
		return j.dump();
	}

	PhysicsComponent PhysicsComponent::DeSerialize(Entity eID, const std::string& data) {
		(void)eID;
		if (data.empty()) {
			return PhysicsComponent();
		}
		nlohmann::json j = nlohmann::json::parse(data);

		PhysicsComponent phys;

		// Deserialize velocity
		phys.vel.x = j.value("velX", 0.0f);
		phys.vel.y = j.value("velY", 0.0f);

		// Deserialize basic properties
		phys.decay = j.value("decay", 0.015f);
		phys.maxSpeed = j.value("maxSpeed", 500.0f);
		phys.isStatic = j.value("isStatic", false);
		phys.mass = j.value("mass", 1.0f);
		phys.restitution = j.value("restitution", 0.5f);
		phys.friction = j.value("friction", 0.1f);

		// BACKWARD COMPATIBILITY: If old JSON has collision data, create ColliderComponent automatically
		// Skip if entity already has a ColliderComponent (loaded from JSON)
		Registry& reg = Registry::Get();
		if ((j.contains("aabb") || j.contains("circle") || j.contains("willCollide")) &&
			!reg.Has_Component<ColliderComponent>(eID)) {

			// Create AABB from old data
			AABB aabb;
			if (j.contains("aabb")) {
				const auto& aabbJson = j["aabb"];
				Vec2 center = {
					aabbJson.value("centerX", 0.0f),
					aabbJson.value("centerY", 0.0f)
				};
				Vec2 dimensions = {
					aabbJson.value("dimX", 0.0f),
					aabbJson.value("dimY", 0.0f)
				};
				aabb = Create_AABB(center, dimensions);
			}

			// Create Circle from old data
			Circle circle;
			if (j.contains("circle")) {
				const auto& circleJson = j["circle"];
				circle.pos.x = circleJson.value("posX", 0.0f);
				circle.pos.y = circleJson.value("posY", 0.0f);
				circle.rad = circleJson.value("rad", 0.0f);
			}

			// Create and add ColliderComponent
			bool willCollide = j.value("willCollide", true);
			ColliderComponent collider(aabb, circle, willCollide);
			reg.Add_Component<ColliderComponent>(eID, collider);

			std::cout << "[Migration] Converted old Physics collision data to ColliderComponent for entity " << eID << "\n";
		}

		return phys;
	}

	// ================================================================================
	// EDITOR
	// ================================================================================

	Editor::ComponentData PhysicsComponent::EditorData() {
		Editor::ComponentData ret(Registry::Get().Get_ComponentTypeName(typeid(PhysicsComponent)));

		ret.Add_Data("Velocity X", Editor::Widget_Float{ &vel.x });
		ret.Add_Data("Velocity Y", Editor::Widget_Float{ &vel.y });
		ret.Add_Data("Decay", Editor::Widget_Float{ &decay, true, 0.0f, 1.0f });
		ret.Add_Data("Max Speed", Editor::Widget_Float{ &maxSpeed, true, 0.f, 2000.f });
		ret.Add_Data("Mass", Editor::Widget_Float{ &mass, true, 0.1f, 100.f });
		ret.Add_Data("Restitution", Editor::Widget_Float{ &restitution, true, 0.f, 1.f });
		ret.Add_Data("Friction", Editor::Widget_Float{ &friction, true, 0.f, 1.f });
		ret.Add_Data("Is Static", Editor::Widget_Checkmark{ &isStatic });

		return ret;
	}
}