#pragma once
#include "Remath.h"
#include <vector>
#include "Collision.h"


namespace Physics {


	// linear forces? drag? friction? i wna dies
	class ForceComp {
		Vec2 dir;				// Unit vector of direction
		float mag;				// Magnitude of vector
		Vec2 force;				// Final calculation of vector
		float age;				// Age of force
		float lifetime;			// Lifetime of force (if 0, means it just applies instantly for one frame then disappears)
	public:
		ForceComp();
		ForceComp(Vec2 direction, float magnitude, float duration = 0);
		~ForceComp() = default;		// should be ok... if using normal types without assigned memory


		void Update();

		void Set_Dir(Vec2 direction);
		void Set_Magnitude(float magnitude);
		bool Expired();
		bool IsImpulse();
		Vec2 GetForce();
	};



	class ForceList {
		std::vector<ForceComp> forces;
		Vec2 net_vel;
		Vec2 Update_NetVel(ForceComp force);
	public:
		void Add_Force(ForceComp);
		void Update();
		bool HasForces();
		const Vec2& Get_NetVel() const;
		void Reset();
	};


	// Physics Component
	class PhysicsComp {
		Vec2 vel;
		float decay;

		bool willCollide;
		AABB aabbhitbox;
		Circle circlehitbox;
		ForceList forcelist;
		
		void UpdateForces();
	public:
		PhysicsComp();
		PhysicsComp(float decayval, bool solid, AABB& aabbref, Circle& circleref);

		void AddForce(Vec2 const& direction, float magnitude, float duration = 0);

		void Update();

		void UpdateHitboxes(Vec2 const& pos);

		void Set_Collision(bool set);

		const Vec2& Get_Vel() const;

		const AABB& Get_AABB() const;

		const Circle& Get_Circle() const;

		const bool& CollisionActive() const;

		void Damping();
	};
}

// movement can be like while vel < speed, force set vel...?