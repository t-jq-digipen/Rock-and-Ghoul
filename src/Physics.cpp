/* ========================================================
 * @file		Physics.cpp
 * @author		Tan Jun Quan | t.junquan@digipen.edu
 * @date		29 SEP 2025
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
	
	ForceComp::ForceComp(Vec2 direction, float magnitude, float duration) : dir{ direction }, mag{ magnitude }, force{ 0.f,0.f }, age{}, lifetime{duration}
	{
		dir = RG_Normalize(direction);
		force = RG_Scale(dir, mag);
	}

	void ForceComp::Update()
	{
		force = RG_Scale(dir, mag);
		if (lifetime)
		{ age += g_dt; }
	}

	Vec2 ForceComp::GetForce()
	{ return force; }

	void ForceComp::Set_Dir(Vec2 direction)
	{ dir = RG_Normalize(direction); }

	void ForceComp::Set_Magnitude(float magnitude)
	{ mag = magnitude; }
	
	bool ForceComp::IsImpulse()
	{ return lifetime == 0.f; }

	bool ForceComp::Expired()
	{ return (age >= lifetime); }



	// Force List Management

	// Add Force to ForceList
	void ForceList::Add_Force(ForceComp force)
	{ forces.push_back(force); }

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
				net_vel = RG_Add(net_vel,Update_NetVel(forcecomp));
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
	{ return !forces.empty(); }

	// Get net velocity change
	const Vec2& ForceList::Get_NetVel() const
	{ return net_vel; }

	// Reset ForceList
	void ForceList::Reset()
	{ forces.clear(); }




	// PHYSICS COMPONENT
	// ================================================================================

	// ctors hehe
	PhysicsComponent::PhysicsComponent() : vel{ 0.f,0.f }, decay{ 0.f }, willCollide{ false }, isStatic{ false }, aabbhitbox{ { 0.f, 0.f }, { 0.f,0.f }, {0.f,0.f} }, circlehitbox{ {0.f,0.f}, 0.f } {}

	PhysicsComponent::PhysicsComponent(float decayval, bool solid, AABB& aabbref, Circle& circleref, bool makeStatic) : vel{ 0.f,0.f }, decay{ decayval }, willCollide{ solid }, aabbhitbox{ aabbref }, circlehitbox{ circleref }, isStatic{ makeStatic } {}


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
		{ vel = RG_Scale(vel, std::pow(decay, g_dt)); }
	}

	// GETTERS
	const Vec2& PhysicsComponent::Get_Vel() const
	{ return vel; }

	const AABB& PhysicsComponent::Get_AABB() const
	{ return aabbhitbox; }

	const Circle& PhysicsComponent::Get_Circle() const
	{ return circlehitbox; }

	const bool& PhysicsComponent::CollisionActive() const
	{ return willCollide; }

	// SETTERS
	// set vel by ?? seems dangerous
	// set aabb by size
	// set circle by radius

	void PhysicsComponent::Set_Collision(bool set)
	{ willCollide = set; }

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

	std::string PhysicsComponent::Serialize() const {
		json j = {
			{"velX", vel.x},
			{"velY", vel.y},
			{"decay", decay},
			{"willCollide", willCollide},
			{"aabb", {
				{"centerX", (aabbhitbox.min.x + aabbhitbox.max.x) * 0.5f},
				{"centerY", (aabbhitbox.min.y + aabbhitbox.max.y) * 0.5f},
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

	PhysicsComponent PhysicsComponent::DeSerialize(const std::string& data) const {
		json j = json::parse(data);

		Vec2 center = { j["aabb"]["centerX"], j["aabb"]["centerY"] };
		Vec2 size = { j["aabb"]["dimX"], j["aabb"]["dimY"] };
		AABB aabb = Create_AABB(center, size);

		Circle circle;
		circle.pos.x = j["circle"]["posX"];
		circle.pos.y = j["circle"]["posY"];
		circle.rad = j["circle"]["rad"];

		return PhysicsComponent(j["decay"], j["willCollide"], aabb, circle);
	}

}