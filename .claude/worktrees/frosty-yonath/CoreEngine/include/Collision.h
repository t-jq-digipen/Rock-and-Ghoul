/* ========================================================
 * @file        Collision.h
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief       Collision detection primitives and algorithms.
 *
 * Defines AABB, Circle, Line, LineSegment and provides collision detection functions for various shape combinations.
 *
 * Supports both static overlap testing and dynamic collision
 * detection. Dynamic checks calculate the exact time when moving objects
 * first collide within a frame
 *
 * Helper functions Create_AABB and Update_AABB manage bounding box
 * construction and position updates for physics systems.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */
#pragma once
#include "Remath.h"
#include <math.h>

 /**
  * @brief Axis-Aligned Bounding Box for 2D collision detection.
  */
struct AABB {
    Vec2 min;   // bottom-left corner
    Vec2 max;   // top-right corner
    Vec2 dim;   // width/height (stored for json)
};

/**
 * @brief Creates an AABB centered at the given position with specified size.
 *
 * @param pos The center position of the bounding box.
 * @param size The dimensions (width, height) of the bounding box.
 * @return AABB The constructed bounding box with calculated min/max corners.
 */
AABB Create_AABB(Vec2 const& pos, Vec2 const& size);

/**
 * @brief Updates an AABB's min/max corners based on new center position.
 *
 * Preserves the original dimensions while recalculating corner positions.
 *
 * @param aabb The bounding box to update (modified in place).
 * @param pos The new center position.
 */
void Update_AABB(AABB& aabb, Vec2 const& pos);

/**
 * @brief Circle primitive for 2D collision detection.
 */
struct Circle {
    Vec2 pos;   // center position
    float rad = 0.f;  // radius of circle
};

/**
 * @brief Infinite line defined by a point and direction.
 */
struct Line {
    Vec2 point;      
    Vec2 direction;  // direction vector (should be normalized)
};

/**
 * @brief Finite line segment defined by two endpoints.
 */
struct LineSegment {
    Vec2 start, end;
};

/**
 * @brief Axis-Aligned Bounding Box for 3D collision detection.
 */
struct AABB3D {
    Vec3 min;
    Vec3 max;
};

/**
 * @brief Finite line segment in 3D space.
 */
struct LineSegment3D {
    Vec3 start, end;
};

// ==============================================
// AABB COLLISION DETECTION
// ==============================================

/**
 * @brief Tests if two AABBs are currently overlapping.
 *
 * @param aabb1 The first bounding box.
 * @param aabb2 The second bounding box.
 * @return bool True if boxes overlap, false otherwise.
 */
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);

/**
 * @brief Tests if two moving AABBs will collide within one frame.
 *
 *
 * @param aabb1 The first bounding box.
 * @param vel1 Velocity of the first box.
 * @param aabb2 The second bounding box.
 * @param vel2 Velocity of the second box.
 * @param firstTimeOfCollision Output parameter for collision time (0.0-0.016).
 * @return bool True if collision occurs within frame, false otherwise.
 */
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision);

// ==============================================
// CIRCLE COLLISION DETECTION
// ==============================================

/**
 * @brief Tests if two circles are currently overlapping.
 *
 * @param c1 The first circle.
 * @param c2 The second circle.
 * @return bool True if circles overlap, false otherwise.
 */
bool CollisionIntersection_CC_Static(const Circle& c1, const Circle& c2);

/**
 * @brief Tests if two moving circles will collide within one frame.
 *
 * @param c1 The first circle.
 * @param vel1 Velocity of the first circle.
 * @param c2 The second circle.
 * @param vel2 Velocity of the second circle.
 * @param firstTimeOfCollision Output parameter for collision time (0.0-0.016).
 * @return bool True if collision occurs within frame, false otherwise.
 */
bool CollisionIntersection_CC_Dynamic(const Circle& c1,
    const Vec2& vel1,
    const Circle& c2,
    const Vec2& vel2,
    float& firstTimeOfCollision);

// ==============================================
// MIXED SHAPE COLLISION DETECTION
// ==============================================

/**
 * @brief Tests if a circle is currently overlapping with an AABB.
 *
 * Uses closest point on AABB to determine overlap.
 *
 * @param circle The circle to test.
 * @param aabb The bounding box to test.
 * @return bool True if shapes overlap, false otherwise.
 */
bool CollisionIntersection_CA_Static(const Circle& circle, const AABB& aabb);

/**
 * @brief Tests if a circle is intersecting with a line segment.
 *
 * Projects circle center onto line segment and checks distance.
 *
 * @param circle The circle to test.
 * @param line The line segment to test.
 * @return bool True if circle intersects line segment, false otherwise.
 */
bool CollisionIntersection_CL_Static(const Circle& circle, const LineSegment& line);

// ==============================================
// 3D COLLISION DETECTION
// ==============================================

/**
 * @brief Tests if two 3D AABBs are currently overlapping.
 *
 * @param a The first 3D bounding box.
 * @param b The second 3D bounding box.
 * @return bool True if boxes overlap, false otherwise.
 */
bool CollisionIntersection_AABB3D_Static(const AABB3D& a, const AABB3D& b);