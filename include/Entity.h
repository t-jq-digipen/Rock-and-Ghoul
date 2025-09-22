
#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include "Remath.h"

using EntityID = uint32_t;
using ComponentMask = std::bitset<32>;

class Entity
{
public:

	// ctor
	Entity(EntityID id);


};


class EntityManager 
{
private:
	EntityID next_entID = 0;
	std::vector<ComponentMask> ent_masks;

public:
	Entity CreateEntity();

};