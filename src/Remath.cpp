#include "Remath.h"

float DistanceSquared(const Vec2& p1, const Vec2& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return dx * dx + dy * dy;
}

float DotProduct(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
}

Vec2 Add(const Vec2& a, const Vec2& b) {
    return { a.x + b.x, a.y + b.y };
}

Vec2 Subtract(const Vec2& a, const Vec2& b) {
    return { a.x - b.x, a.y - b.y };
}

Vec2 Scale(const Vec2& v, float scalar) {
    return { v.x * scalar, v.y * scalar };
}
