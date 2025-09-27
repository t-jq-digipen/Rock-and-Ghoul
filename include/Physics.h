#pragma once
#include "Remath.h"
#include <vector>
#include "CoreEngine.h"


namespace Physics {


	// linear forces? drag? friction? i wna die


	class ForceComp {
	protected:
		Vec2 dir;				// Unit vector of direction
		float mag;				// Magnitude of vector
		Vec2 vel;				// Final calculation of vector
		float age;				// Age of force
		float lifetime;			// Lifetime of force (if 0, means it just applies instantly for one frame then disappears)
	public:
		ForceComp();
		ForceComp(Vec2 direction, float magnitude, float lifetime = 0);
		~ForceComp() = default;		// should be ok... if using normal types without assigned memory

		void Update();				
		bool Expired();
		Vec2 GetVel();
	};



	class ForceList {
		std::vector<ForceComp> forcelist;
		Vec2 net_vel;
	public:
		void Add_Force(ForceComp);
		void Remove_Force(ForceComp);
		void Update();
	};


	// Physics Component
	class PhysicsComp {
		Vec2 vel;

	};
}

// movement can be like while vel < speed, force set vel...?