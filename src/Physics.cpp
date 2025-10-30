/* ========================================================
 * @file		Physics.cpp
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @co-author	Yasmin Tazi	 | yasmin.t@digipen.edu
 * @date		30 OCT 2025
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
		, decay{ 0.f }
		, isStatic{ false }
		, willCollide{ false }
		, aabbhitbox{ { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f } }
		, circlehitbox{ { 0.f, 0.f }, 0.f }
		, mass{ 1.f }
		, restitution{ 0.5f }
	{
		forces.reserve(8); // Reserve space for common case
	}

	PhysicsComponent::PhysicsComponent(float decayval, bool solid, AABB& aabbref, Circle& circleref, bool makeStatic)
		: vel{ 0.f, 0.f }
		, decay{ decayval }
		, isStatic{ makeStatic }
		, willCollide{ solid }
		, aabbhitbox{ aabbref }
		, circlehitbox{ circleref }
		, mass{ 1.f }
		, restitution{ 0.5f }
	{
		forces.reserve(8);
	}

	// ================================================================================
	// FORCE MANAGEMENT
	// ================================================================================

	void PhysicsComponent::AddForce(Vec2 const& direction, float magnitude, float duration) {
		forces.emplace_back(direction, magnitude, duration);
	}

	void PhysicsComponent::ClearForces() {
		forces.clear();
	}

	bool PhysicsComponent::HasForces() const {
		return !forces.empty();
	}

	void PhysicsComponent::UpdateForces(float dt) {
		if (forces.empty()) {
			return;
		}

		Vec2 netVelChange{ 0.f, 0.f };

		// Single pass: update forces and accumulate velocity changes
		for (auto it = forces.begin(); it != forces.end(); ) {
			it->Update(dt);

			// Calculate velocity contribution
			const Vec2& force = it->GetForce();
			if (it->IsImpulse()) {
				netVelChange = RG_Add(netVelChange, force);
			}
			else {
				netVelChange = RG_Add(netVelChange, RG_Scale(force, dt));
			}

			// Remove if expired
			if (it->Expired()) {
				it = forces.erase(it);
			}
			else {
				++it;
			}
		}

		// Apply accumulated velocity change
		vel = RG_Add(vel, netVelChange);
	}

	// ================================================================================
	// UPDATE METHODS
	// ================================================================================

	void PhysicsComponent::ApplyDamping(float dt) {
		if (decay > 0.f) {
			vel = RG_Scale(vel, std::pow(decay, dt));
		}
	}

	void PhysicsComponent::Update(float dt) {
		ApplyDamping(dt);
		UpdateForces(dt);
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
	// STATIC/DYNAMIC CONTROL
	// ================================================================================

	bool PhysicsComponent::IsStatic() const {
		return isStatic;
	}

	void PhysicsComponent::SetStatic(bool s) {
		isStatic = s;
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

	void PhysicsComponent::SetMass(float m) {
		mass = m;
	}

	void PhysicsComponent::SetRestitution(float e) {
		restitution = std::max(0.f, std::min(1.f, e));
	}

	// ================================================================================
	// IMPULSE PHYSICS
	// ================================================================================

	void PhysicsComponent::ApplyLinearImpulse(const Vec2& impulse) {
		if (!isStatic) {
			Vec2 deltaV = RG_Scale(impulse, 1.0f / mass);
			vel = RG_Add(vel, deltaV);
		}
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
			{"isStatic", isStatic},
			{"willCollide", willCollide},
			{"mass", mass},
			{"restitution", restitution},
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
		phys.isStatic = j.value("isStatic", false);
		phys.willCollide = j.value("willCollide", false);
		phys.mass = j.value("mass", 1.0f);
		phys.restitution = j.value("restitution", 0.5f);

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

	Editor::ComponentData PhysicsComponent::EditorData() {
		Editor::ComponentData ret(Registry::Get().Get_ComponentTypeName(typeid(PhysicsComponent)));

		ret.Add_Data("WillCollide", Editor::Widget_Checkmark{ &willCollide });

		ret.Add_Data("AABB width", Editor::Widget_Float{ &aabbhitbox.dim.x, true, 10.f, 1000.f });
		ret.Add_Data("AABB height", Editor::Widget_Float{ &aabbhitbox.dim.y, true, 10.f, 1000.f });

		ret.Add_Data("Circle radius", Editor::Widget_Float{ &circlehitbox.rad, true, 10.f, 500.f });

		return ret;
	}
}