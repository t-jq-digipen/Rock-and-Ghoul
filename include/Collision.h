#pragma once
#ifndef COLLISION_H
#define COLLISION_H

struct Vec2 {
    float x, y;
};

struct AABB {
    Vec2 min; // bottom-left corner
    Vec2 max; // top-right corner
};

// Static AABB check
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);

// Dynamic AABB check
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision);

#endif // COLLISION_H
