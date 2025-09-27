#include "Physics.h"

namespace Physics {
	// Force Component

	// Ctor
	ForceComp::ForceComp() : dir{ 0.f,0.f }, mag{ 0.f }, vel{ 0.f,0.f }, age{}, lifetime{} {}
	
	ForceComp::ForceComp(Vec2 direction, float magnitude, float duration) : dir{ direction }, mag{ magnitude }, vel{ 0.f,0.f }, age{}, lifetime{duration}
	{
		dir = Normalize(direction);
		vel = Scale(dir, mag);
	}

	void ForceComp::Update()
	{
		vel = Scale(dir, mag);
		if (lifetime)
		{ age += g_dt; }
	}

	Vec2 ForceComp::GetVel()
	{
		return vel;
	}

	// Set dir
	// Set mag

	bool ForceComp::Expired()
	{
		return (age > lifetime);
	}


	// Force List Management
	void ForceList::Update()
	{
		// Update forces
		for (ForceComp& force : forcelist)
		{
			force.Update();
		}

		// Remove expired
		for (auto force = forcelist.begin(); force != forcelist.end();)
		{
			if (force->Expired())
			{
				// Erase, return to next valid iterator
				force = forcelist.erase(force);
			}
			else
			{
				// Go to next iterator if not erased
				force++;
			}
		}

		// Add final

		
	}
}