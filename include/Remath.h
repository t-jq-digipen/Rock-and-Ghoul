#pragma once

struct Vec2 {
    float x, y;
};
// Calculate squared distance between 2 points
float DistanceSquared(const Vec2& p1, const Vec2& p2);
// Calculate dot product between 2 vectors
float DotProduct(const Vec2& a, const Vec2& b);
// Addition between 2 vectors
Vec2 Add(const Vec2& a, const Vec2& b);
// Subtraction between 2 vectors
Vec2 Subtract(const Vec2& a, const Vec2& b);
// Scaling a vector
Vec2 Scale(const Vec2& v, float scalar);

