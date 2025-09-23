#include "Collision.h"
#include <math.h>
#include "Remath.h"
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
    Vec2 relVel = Vec2{ vel1.x - vel2.x, vel1.y - vel2.y };

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

// Circle Collision
bool CollisionIntersection_CC_Static(const Circle& c1, const Circle& c2) {
    float dx = c2.x - c1.x;
    float dy = c2.y - c1.y;
    float d2 = (dx * dx) + (dy * dy);

    float radSum = c1.rad + c2.rad;
    float rad2 = radSum * radSum; // MIGHT BE WRONG CHECK LATER 

    return (d2 <= rad2);
}

bool CollisionIntersection_CC_Dynamic(const Circle& c1,
    const Vec2& vel1,
    const Circle& c2,
    const Vec2& vel2,
    float& firstTimeOfCollision) {

    if (CollisionIntersection_CC_Static(c1, c2)) {
        firstTimeOfCollision = 0.0f;
        return true;
    }

    Vec2 relVel = Vec2{ vel1.x - vel2.x, vel1.y - vel2.y };

    float tFirst = 0.0f;
    float tLast = 0.016f;

    // speed at which circles are schmovinnnn
    float relSpeed = sqrt((relVel.x * relVel.x) + (relVel.y * relVel.y)); // USING MATH LIBRARY REMOVE LATER AND USE CUSTOM MATH ENGINE
    if (relSpeed == 0.0f) {
        return false;
    }

    // vec of diff between c1 and c2
    Vec2 centerDiff = Vec2{ c2.x - c1.x, c2.y - c1.y };

    // project the vec onto the direction that circle is going??? 
    float proj = (centerDiff.x * relVel.x + centerDiff.y * relVel.y) / relSpeed;
    if (proj <= 0) {
        return false;
    }

    float distance = (centerDiff.x * centerDiff.x + centerDiff.y * centerDiff.y) - (proj * proj);

    float radSum = c1.rad + c2.rad;
    float rad2 = radSum * radSum;

    if (radSum < distance) {
        return false;
    }
}

bool CollisionIntersection_CL_Static(const Circle& circle, const LineSegment& line) {
        // starting from point on line to center of circle
        Vec2 linecircle = Subtract(Vec2{ circle.x, circle.y }, line.start);

        // dir vector of line
        Vec2 vecDir = Subtract(line.end, line.start);

        // squared length of the line segment
        float lineLengthSquared = DotProduct(vecDir, vecDir);

        //// Handle degenerate case (line segment is actually a point)
        //if (lineLengthSquared == 0.0f) {
        //    return DistanceSquared({ circle.x, circle.y }, line.start) <= (circle.rad * circle.rad);
        //}

        // circle center projected on line segment
        float t = DotProduct(linecircle, vecDir) / lineLengthSquared;

        // clamp t to [0, 1] to stay within the line segment
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;

        // find the closest point on the line segment for collision and scale with the clamp
        Vec2 closestPoint = Add(line.start, Scale(vecDir, t));

        // check if circle intersects with the closest point
        float distanceSquared = DistanceSquared(Vec2{ circle.x, circle.y }, closestPoint);

        return distanceSquared <= (circle.rad * circle.rad);
    
}

bool CollisionIntersection_AABB3D_Static(const AABB3D& a, const AABB3D& b) {
    if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
    if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
    if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
    return true;
}