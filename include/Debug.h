/* ========================================================
 * @file        Debug.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * Provides debug rendering utilities for visualizing physics and collision components.
 * 
 * The DebugDraw namespace contains helper functions for drawing
 * shapes and visual for AABBs, velocity vectors, and
 * circle colliders.
 *
 * Functions include:
 * - DrawAABB:           Renders an axis-aligned bounding box.
 * - DrawHotspot:        Marks a point of interest (e.g., collider center).
 * - DrawVelocity:       Renders a velocity vector for an entity.
 * - DrawCircleCollider: Renders a circle collider outline.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include "Render.h"
#include "Physics.h" // for AABB, PhysicsComponent
#include "Collision.h"
#include "Component.h"
#include "Registry.h"
#include "SpatialGrid.h"
namespace DebugDraw {
    /**
    * @brief Draw an axis-aligned bounding box (AABB).
    * @param box The AABB to draw.
    * @param renderer The renderer used to draw the shape.
    */
    void DrawAABB(const AABB& box, Render& renderer);

    /**
    * @brief Draw a small hotspot/marker at a given position.
    * @param pos The world-space position to mark.
    * @param renderer The renderer used to draw the hotspot.
    */
    void DrawHotspot(const Vec2& pos, Render& renderer);

    /**
     * @brief Draw the velocity vector of an entity for debugging.
     * @param e The entity whose velocity is being drawn.
     * @param t The transform component (for position reference).
     * @param renderer The renderer used to draw the vector.
     * @param dt The delta time used to scale the velocity.
     */
    void DrawVelocity(Entity e, const TransformComponent& t, Render& renderer, float dt);

    /**
     * @brief Draw the circle collider.
     * @param circle The circle collider to draw.
     * @param renderer The renderer used to draw the circle.
     */
    void DrawCircleCollider(const Circle& circle, Render& renderer);

    void DrawSpatialGrid(SpatialGrid* grid, Render& renderer);
}

