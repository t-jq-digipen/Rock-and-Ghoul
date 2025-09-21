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

struct Circle {
    float x, y; // centre of circle
    float rad; // radius of circle
};

struct Line {
    Vec2 point, direction; // direction vector should be normalized
    
};

struct LineSegment {
    Vec2 start, end; // note segments for fretboard
};

// Static AABB check
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);

// Dynamic AABB check
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision);

// Static Circle-Circle collision check
bool CollisionIntersection_CC_Static(const Circle& c1, const Circle& c2);

// Dynamic Circle-Circle collision check
bool CollisionIntersection_CC_Dynamic(const Circle& c1,
    const Vec2& vel1,
    const Circle& c2,
    const Vec2& vel2,
    float& firstTimeOfCollision);

// Static Circle-Line collision check
bool CollisionIntersection_CL_Static(const Circle& circle, const LineSegment& line);

// Dynamic Circle-Line collision check
// No need for now because the circles will be on the fretboard in fixed locations and unmoving
//bool CollisionIntersection_CL_Dynamic(const Circle& circle,
//    const Vec2 vel1,
//    const Line& line,
//    const Vec2& vel2,
//    float& firstTimeOfCollision);


#endif // COLLISION_H
