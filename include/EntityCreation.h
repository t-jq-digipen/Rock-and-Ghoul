/* ========================================================
 * @file    EntityCreation.h
 * @author  w.shiyuregine@digipen.edu
 * @date    01/10/2025
 *
 * @brief
 *
 * This file centralizes entity construction logic to ensure consistency
 * when spawning game objects. Each creation function initializes a new
 * entity, attaches the required components (transform, physics, sprite,
 * input, health, etc.), and sets defaults such as position,
 * scale, collision bounds, and textures.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "EntityManager.h"
#include "Render.h"
#include "Registry.h"
//Entity Create_Player(float x, float y);
//Entity Create_Square(float x, float y);
//Entity Create_Circle(float x, float y);
// Entity Create_Ghoul(float x, float y);
#include "Component.h"
Entity Create_Player(float const& spawnx, float const& spawny);
Entity Create_Waffle(float const& spawnx, float const& spawny);
/**
 * @brief Create a generic Shape entity (rectangle or circle) with transform,
 *        physics, shape, input, tag, and health components.
 * @param spawnx The initial x-position of the shape.
 * @param spawny The initial y-position of the shape.
 * @param shapeType The shape type (rectangle or circle).
 * @param controls The control scheme used to move the shape.
 * @return The newly created Shape entity.
 */
Entity Create_Shape(float const& spawnx, float const& spawny, ShapeType shapeType, ControlScheme controls);
Entity Create_Wall(float x, float y, float width, float height);