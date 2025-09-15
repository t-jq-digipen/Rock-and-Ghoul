#include "Collision.h"

// Static collision
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2)
{
    if (aabb1.max.x < aabb2.min.x || aabb1.min.x > aabb2.max.x) return false;
    if (aabb1.max.y < aabb2.min.y || aabb1.min.y > aabb2.max.y) return false;
    return true;
}

// Dynamic collision
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision)
{
    // Step 1: static check
    if (CollisionIntersection_RectRect_Static(aabb1, aabb2)) {
        firstTimeOfCollision = 0.0f;
        return true;
    }

    // Step 2: relative velocity
    Vec2 relVel = { vel1.x - vel2.x, vel1.y - vel2.y };

    float tFirst = 0.0f;
    float tLast = 0.016f; // simulate within one frame (60 FPS)

    for (int i = 0; i < 2; ++i) {
        float aMin, aMax, bMin, bMax, vel;
        if (i == 0) { // X axis
            aMin = aabb1.min.x; aMax = aabb1.max.x;
            bMin = aabb2.min.x; bMax = aabb2.max.x;
            vel = relVel.x;
        }
        else { // Y axis
            aMin = aabb1.min.y; aMax = aabb1.max.y;
            bMin = aabb2.min.y; bMax = aabb2.max.y;
            vel = relVel.y;
        }

        if (vel == 0) {
            if (aMax < bMin || aMin > bMax) return false;
            continue;
        }

        float tEnter, tExit;
        if (vel > 0) {
            tEnter = (bMin - aMax) / vel;
            tExit = (bMax - aMin) / vel;
        }
        else {
            tEnter = (bMax - aMin) / vel;
            tExit = (bMin - aMax) / vel;
        }

        if (tEnter > tFirst) tFirst = tEnter;
        if (tExit < tLast)  tLast = tExit;
        if (tFirst > tLast || tLast < 0.0f) return false;
    }

    firstTimeOfCollision = tFirst;
    return (tFirst >= 0.0f && tFirst <= 0.016f);
}
