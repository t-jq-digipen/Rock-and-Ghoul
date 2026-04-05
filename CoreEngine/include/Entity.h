/* ========================================================
 * @file		Entity.h
 * @author		Tan Jun Quan		| t.junquan@digipen.edu 50%
 * @co-author	Wong Shi Yu Regine  | w.shiyuregine@digipen.edu 50%
 * @date		29 SEP 2025
 *
 * @brief	Entity type declaration for ECS
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
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
