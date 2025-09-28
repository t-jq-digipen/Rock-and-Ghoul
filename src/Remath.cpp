#include "Remath.h"
#include <cmath>

#pragma region TRIGO

// ==============================================
// HELPER FUNCTION: Get PI constant for any type
// ==============================================
template<typename T>
T RG_GetPI() {
    return (T)RG_PI;
}

// ==============================================
// ANGLE CONVERSION
// ==============================================

// Degree to Radians
template<typename T>
T RG_DegToRad(T x) {
    return x * (RG_GetPI<T>() / (T)180.0);
}

// Radians to Degree
template<typename T>
T RG_RadToDeg(T x) {
    return x * ((T)180.0 / RG_GetPI<T>());
}

// ==============================================
// TRIGO FUNCTIONS
// ==============================================
template<typename T>
T RG_Sin(T x) {
    return (T)sin((double)x);
}

template<typename T>
T RG_Cos(T x) {
    return (T)cos((double)x);
}

template<typename T>
T RG_Tan(T x) {
    return (T)tan((double)x);
}

template<typename T>
// Clamping x to the domain of asin (-1, 1)
T RG_ASin(T x) {
    if (x < (T)-1.0) {
        x = (T)-1.0;
    }
    if (x > (T)1.0) {
        x = (T)1.0;
    }
    return (T)asin((double)x);
}

template<typename T>
// Clamping x to domain of acos (-1, 1)
T RG_ACos(T x) {
    if (x < (T)-1.0) {
        x = (T)-1.0;
    }
    if (x > (T)1.0) {
        x = (T)1.0;
    }
    return (T)acos((double)x);
}

template<typename T>
T RG_ATan(T x) {
    return (T)atan((double)x);
}

template<typename T>
T RG_ATan2(T y, T x) {
    return (T)atan2((double)y, (double)x);
}

// ==============================================
// DEGREE VARIANTS
// ==============================================

template<typename T>
T RG_SinDeg(T x) {
    return RG_Sin(RG_DegToRad(x));
}

template<typename T>
T RG_CosDeg(T x) {
    return RG_Cos(RG_DegToRad(x));
}

template<typename T>
T RG_TanDeg(T x) {
    return RG_Tan(RG_DegToRad(x));
}

template<typename T>
T RG_ASinDeg(T x) {
    return RG_RadToDeg(RG_ASin(x));
}

template<typename T>
T RG_ACosDeg(T x) {
    return RG_RadToDeg(RG_ACos(x));
}

template<typename T>
T RG_ATanDeg(T x) {
    return RG_RadToDeg(RG_ATan(x));
}

float RG_DistanceSquared(const Vec2& p1, const Vec2& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return dx * dx + dy * dy;
}

float RG_DotProduct(const Vec2& a, const Vec2& b) {
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

// Normalize
Vec2 Normalize(const Vec2 a)
{
    const float len = std::sqrt(a.x * a.x + a.y * a.y);

    if (len>0.f)
    { return { a.x / len, a.y / len }; }

    return { 0.f,0.f };
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