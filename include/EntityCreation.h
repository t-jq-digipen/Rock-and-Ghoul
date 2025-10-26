/* ========================================================
 * @file		EntityCreation.h
 * @author      Wong Shi Yu Regine  | w.shiyuregine@digipen.edu
 * @co-author   Tan Jun Quan        | t.junquan@digipen.edu
 * @co-author   Yasmin Tazi         | yasmin.t@digipen.edu
 * @date		01 OCT 2025
 *
 * @brief
 *
 * This file centralizes entity construction logic to ensure consistency
 * when spawning game objects. Each creation function initializes a new
 * entity, attaches the required components (transform, physics, sprite,
 * input, health, etc.), and sets defaults such as position,
 * scale, collision bounds, and textures.
 *
 * Copyright � 2025 DigiPen, All rights reserved.
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

/**
 * @brief Create a player entity with transform,
 *        physics, shape, input, tag, and health components.
 * @param spawnx The initial x-position of the shape.
 * @param spawny The initial y-position of the shape.
 * @return The newly created Player entity.
 */
Entity Create_Player(float const& spawnx, float const& spawny);
/**
 * @brief Create a waffle entity with transform,
 *        physics, shape, input, tag, and health components.
 * @param spawnx The initial x-position of the shape.
 * @param spawny The initial y-position of the shape.
 * @return The newly created waffle entity. For engine testing.
 */
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
/**
 * @brief Create a wall entity with transform, physics, shape, and tag components.
 * @param x The x-position of the wall.
 * @param y The y-position of the wall.
 * @param width The width of the wall in world units.
 * @param height The height of the wall in world units.
 * @return The newly created Wall entity.
 */
Entity Create_Wall(float x, float y, float width, float height);
/**
 * @brief Create a lightweight instanced square entity with transform, shape, and tag components.
 * @param x The x-position of the square.
 * @param y The y-position of the square.
 * @param size The size (width and height) of the square.
 * @param color The RGB color of the square.
 * @return The newly created instanced square entity.
 */
Entity Create_InstancedSquare(float x, float y, float size, Vec3 color);

/**
 * @brief Generates initial prefab JSON files in Assets/prefabs/.
 *
 * Creates the prefabs directory if missing and generates default
 * entity prefabs (Player, Waffle, Rectangle, Circle). 
 *
 */
void GenerateInitialPrefabs();


Entity Create_RotatingObject(float x, float y);
Entity Create_MovingObject(float x, float y);

