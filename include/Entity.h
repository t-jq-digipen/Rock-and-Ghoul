
#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include "Remath.h"

using Entity = uint32_t;
//if 0 means cannot ever use entity 0 as a real entity ID
//-1 means "no entity" = the maximum possible value for the type
//all free to be valid entity IDs
constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1); // 4294967295
