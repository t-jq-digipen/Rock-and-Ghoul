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

// Linear interpolation: find where u are along a straight line
Vec2 Lerp(const Vec2& a, const Vec2& b, float t) {
    return Add(a, Scale(Subtract(b, a), t));
}

float Sqrt(float x) {
    if (x < 0) { return 0; } // cannot sqrt -ve number

    if (x == 0 || x == 1) { return x; } // special cases

    float precision = 0.00001; // accurate to 5dp
    float guess = x * 0.5f;
    float abs;

    for (int i = 0; i < 10; ++i) { // capping iterations
        float newGuess = 0.5f * (guess + x / guess);

        if ((newGuess - guess) < 0) {
            abs = -(newGuess - guess);
        }
        else { abs = newGuess - guess; }

        // if absolute value of difference between newGuess and Guess is negligible by precision, newGuess is good
        if (abs < precision) {
            return newGuess;
        }

        guess = newGuess; // trying again
    }

    return guess;
}

void Clamp(float& val, float const& min, float const& max)
{
    if (val > max)
    { val = max; }
    if (val < min)
    { val = min; }
}