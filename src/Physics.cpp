/* ========================================================
 * @file		Physics.cpp
 * @author		Yasmin Tazi  | yasmin.t@digipen.edu
 * @co-author	Tan Jun Quan | t.junquan@digipen.edu
 * @date		31 OCT 2025
 *
 * @brief	Physics component for ECS
 *
 * This file implements physics related components and utilities
 * such as managing forces, calculating net velocities and
 * updating an entity based on the forces applied.
 *
 * The bulk of the updates happen in PhysicsSystem.cpp
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "Physics.h"
#include "CoreEngine.h"
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
		, decay{ 0.95f }       // Slight damping
		, maxSpeed{ 500.f }    // Prevent tunneling
		, isStatic{ false }
		, willCollide{ false }
		, aabbhitbox{ { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f } }
		, circlehitbox{ { 0.f, 0.f }, 0.f }
		, mass{ 1.f }
		, restitution{ 0.3f }  // Some bounce
		, friction{ 0.1f }     // Slight friction
		, onGround{ false }
		, collisionFrameCount{ 0 }
		, layerBit{ 0 }
		, maskBit{ 0 }
		, isTrigger{ false }
	{
		forces.reserve(8); 
	}

	PhysicsComponent::PhysicsComponent(float decayval, bool solid, AABB& aabbref, Circle& circleref, bool makeStatic)
		: vel{ 0.f, 0.f }
		, acceleration{ 0.f, 0.f }
		, decay{ decayval }
		, maxSpeed{ 500.f }
		, isStatic{ makeStatic }
		, willCollide{ solid }
		, aabbhitbox{ aabbref }
		, circlehitbox{ circleref }
		, mass{ 1.f }
		, restitution{ 0.3f }
		, friction{ 0.1f }
		, onGround{ false }
		, collisionFrameCount{ 0 }
		, layerBit{ 0 }
		, maskBit{ 0 }
		, isTrigger{ false }
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

	void PhysicsComponent::UpdateHitboxes(Vec2 const& pos) {
		Update_AABB(aabbhitbox, pos);
		circlehitbox.pos = pos;
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
	// COLLISION GETTERS/SETTERS
	// ================================================================================

	void PhysicsComponent::Set_Collision(bool set) {
		willCollide = set;
	}

	const bool& PhysicsComponent::CollisionActive() const {
		return willCollide;
	}

	// ================================================================================
	// HITBOX GETTERS/SETTERS
	// ================================================================================

	const AABB& PhysicsComponent::Get_AABB() const {
		return aabbhitbox;
	}

	const Circle& PhysicsComponent::Get_Circle() const {
		return circlehitbox;
	}

	void PhysicsComponent::Set_AABB(AABB const& newAABB) {
		aabbhitbox = newAABB;
	}

	void PhysicsComponent::Set_Circle(Circle const& newCircle) {
		circlehitbox = newCircle;
	}

	// ================================================================================
	// FILTER GETTERS/SETTERS
	// ================================================================================

	void PhysicsComponent::SetLayer(uint32_t bit) {
		layerBit = bit;
	}

	void PhysicsComponent::SetMask(uint32_t mask) {
		maskBit = mask;
	}

	void PhysicsComponent::SetTrigger(bool trigger) {
		isTrigger = trigger;
	}

	uint32_t PhysicsComponent::GetLayer() {
		return layerBit;
	}

	uint32_t PhysicsComponent::GetMask() {
		return maskBit;
	}

	bool PhysicsComponent::IsTrigger() {
		return isTrigger;
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
		// Calculate center from AABB for saving
		float centerX = (aabbhitbox.min.x + aabbhitbox.max.x) * 0.5f;
		float centerY = (aabbhitbox.min.y + aabbhitbox.max.y) * 0.5f;

		nlohmann::json j = {
			{"velX", vel.x},
			{"velY", vel.y},
			{"decay", decay},
			{"maxSpeed", maxSpeed},
			{"isStatic", isStatic},
			{"willCollide", willCollide},
			{"mass", mass},
			{"restitution", restitution},
			{"friction", friction},
			{"aabb", {
				{"centerX", centerX},
				{"centerY", centerY},
				{"dimX", aabbhitbox.dim.x},
				{"dimY", aabbhitbox.dim.y}
			}},
			{"circle", {
				{"posX", circlehitbox.pos.x},
				{"posY", circlehitbox.pos.y},
				{"rad", circlehitbox.rad}
			}}
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
		phys.willCollide = j.value("willCollide", false);
		phys.mass = j.value("mass", 1.0f);
		phys.restitution = j.value("restitution", 0.5f);
		phys.friction = j.value("friction", 0.1f);

		// Deserialize AABB
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
			phys.aabbhitbox = Create_AABB(center, dimensions);
		}

		// Deserialize Circle
		if (j.contains("circle")) {
			const auto& circleJson = j["circle"];
			phys.circlehitbox.pos.x = circleJson.value("posX", 0.0f);
			phys.circlehitbox.pos.y = circleJson.value("posY", 0.0f);
			phys.circlehitbox.rad = circleJson.value("rad", 0.0f);
		}

		return phys;
	}

	// ================================================================================
	// EDITOR
	// ================================================================================

	// NEED TO ADD WIDGET STUFF
	Editor::ComponentData PhysicsComponent::EditorData() {
	Editor::ComponentData ret(Registry::Get().Get_ComponentTypeName(typeid(PhysicsComponent)));

		ret.Add_Data("WillCollide", Editor::Widget_Checkmark{ &willCollide });

		ret.Add_Data("AABB width", Editor::Widget_Float{ &aabbhitbox.dim.x, true, 10.f, 1000.f });
		ret.Add_Data("AABB height", Editor::Widget_Float{ &aabbhitbox.dim.y, true, 10.f, 1000.f });

		ret.Add_Data("Circle radius", Editor::Widget_Float{ &circlehitbox.rad, true, 10.f, 500.f });

		return ret;
	}
}