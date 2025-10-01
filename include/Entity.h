/* ========================================================
 * @file	Entity.h
 * @author	Tan Jun Quan
 * @date	24 SEP 2025
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
using Entity = unsigned int;
constexpr Entity INVALID_ENTITY = static_cast<Entity>(-1); // 4294967295
