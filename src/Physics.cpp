/* ========================================================
 * @file		Physics.cpp
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @co-author	Yasmin Tazi	 | yasmin.t@digipen.edu
 * @date		26 OCT 2025
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

namespace Physics {
	// Force Component

	// Ctor
	ForceComp::ForceComp() : dir{ 0.f,0.f }, mag{ 0.f }, force{ 0.f,0.f }, age{}, lifetime{} {}

	ForceComp::ForceComp(Vec2 direction, float magnitude, float duration) : dir{ direction }, mag{ magnitude }, force{ 0.f,0.f }, age{}, lifetime{ duration }
	{
		dir = RG_Normalize(direction);
		force = RG_Scale(dir, mag);
	}

	void ForceComp::Update()
	{
		force = RG_Scale(dir, mag);
		if (lifetime)
		{
			age += g_dt;
		}
	}

	Vec2 ForceComp::GetForce()
	{
		return force;
	}

	void ForceComp::Set_Dir(Vec2 direction)
	{
		dir = RG_Normalize(direction);
	}

	void ForceComp::Set_Magnitude(float magnitude)
	{
		mag = magnitude;
	}

	bool ForceComp::IsImpulse()
	{
		return lifetime == 0.f;
	}

	bool ForceComp::Expired()
	{
		return (age >= lifetime);
	}



	// Force List Management

	// Add Force to ForceList
	void ForceList::Add_Force(ForceComp force)
	{
		forces.push_back(force);
	}

	Vec2 ForceList::Update_NetVel(ForceComp force)
	{
		if (force.IsImpulse())
		{
			return force.GetForce();
		}
		return RG_Scale(force.GetForce(), g_dt);
	}

	// Update ForceList
	void ForceList::Update()
	{
		// Reset net_vel to 0
		net_vel = { 0.f,0.f };

		if (HasForces())
		{
			// Update force time and magnitude!!!
			for (ForceComp& forcecomp : forces)
			{
				forcecomp.Update();
				// Update net_vel change!
				net_vel = RG_Add(net_vel, Update_NetVel(forcecomp));
			}

			// Remove expired
			for (auto forcecomp = forces.begin(); forcecomp != forces.end();)
			{
				if (forcecomp->Expired())
				{
					// Erase, return to next valid iterator
					forcecomp = forces.erase(forcecomp);
				}
				else
				{
					// Go to next iterator if not erased
					forcecomp++;
				}
			}
		}
	}

	// Check if list has forces
	bool ForceList::HasForces()
	{
		return !forces.empty();
	}

	// Get net velocity change
	const Vec2& ForceList::Get_NetVel() const
	{
		return net_vel;
	}

	// Reset ForceList
	void ForceList::Reset()
	{
		forces.clear();
	}




	// PHYSICS COMPONENT
	// ================================================================================

	// ctors hehe
	PhysicsComponent::PhysicsComponent() :
		vel{ 0.f,0.f },
		decay{ 0.f },
		willCollide{ false },
		isStatic{ false },
		aabbhitbox{ { 0.f, 0.f }, { 0.f,0.f }, {0.f,0.f} },
		circlehitbox{ {0.f,0.f}, 0.f },
		mass(1.f),
		restitution(0.5f)
	{
	}

	PhysicsComponent::PhysicsComponent(float decayval, bool solid, AABB& aabbref, Circle& circleref, bool makeStatic) :
		vel{ 0.f,0.f },
		decay{ decayval },
		willCollide{ solid },
		aabbhitbox{ aabbref },
		circlehitbox{ circleref },
		isStatic{ makeStatic },
		mass(1.f),
		restitution(0.5f)
	{
	}


	// FORCES
	void PhysicsComponent::AddForce(Vec2 const& direction, float magnitude, float duration)
	{
		ForceComp temp(direction, magnitude, duration);
		forcelist.Add_Force(temp);
	}

	void PhysicsComponent::UpdateForces()
	{
		forcelist.Update();
		vel = RG_Add(vel, forcelist.Get_NetVel());
	}



	// COMPONENT STUFF
	void PhysicsComponent::Update()
	{
		Damping();
		UpdateForces();
	}

	void PhysicsComponent::UpdateHitboxes(Vec2 const& pos)
	{
		Update_AABB(aabbhitbox, pos);
		circlehitbox.pos = pos;
	}

	void PhysicsComponent::Damping()
	{
		if (decay)
		{
			vel = RG_Scale(vel, std::pow(decay, g_dt));
		}
	}

	// GETTERS
	const Vec2& PhysicsComponent::Get_Vel() const
	{
		return vel;
	}

	const AABB& PhysicsComponent::Get_AABB() const
	{
		return aabbhitbox;
	}

	const Circle& PhysicsComponent::Get_Circle() const
	{
		return circlehitbox;
	}

	const bool& PhysicsComponent::CollisionActive() const
	{
		return willCollide;
	}

	// SETTERS
	void PhysicsComponent::Set_Collision(bool set)
	{
		willCollide = set;
	}

	void PhysicsComponent::Set_Vel(Vec2 const& newVel)
	{
		vel = newVel;
	}

	void PhysicsComponent::Set_AABB(AABB const& newAABB)
	{
		aabbhitbox = newAABB;
	}

	void PhysicsComponent::Set_Circle(Circle const& newCircle)
	{
		circlehitbox = newCircle;
	}

	// LINEAR IMPULSE METHOD
	void PhysicsComponent::ApplyLinearImpulse(const Vec2& impulse) {
		if (!isStatic) {
			Vec2 deltaV = RG_Scale(impulse, 1.0f / mass);
			vel = RG_Add(vel, deltaV);
		}
	}


	// SERIALIZATION METHODS
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

	Editor::ComponentData PhysicsComponent::EditorData() {
		Editor::ComponentData ret("Physics");

		ret.Add_Data("WillCollide", Editor::Widget_Checkmark{ &willCollide });

		ret.Add_Data("AABB width", Editor::Widget_Float{ &aabbhitbox.dim.x, true, 10.f, 1000.f });
		ret.Add_Data("AABB height", Editor::Widget_Float{ &aabbhitbox.dim.y, true, 10.f, 1000.f });

		ret.Add_Data("Cirle radius", Editor::Widget_Float{ &circlehitbox.rad, true, 10.f, 500.f });

		return ret;
	}
}