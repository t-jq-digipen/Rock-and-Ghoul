/* ========================================================
 * @file        Remath.cpp
 * @author      Yasmin Tazi         | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa   | t.yingclarissa@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 * Implementation file for Remath library. This library includes
 * trigonometric functions, vector operations, and common math utilities.
 *
 * Features:
 * - Trigonometric Functions: Template-based sin, cos, tan, and their
 *   inverse variants (asin, acos, atan, atan2) with built-in clamping
 *   for inverse functions.
 *
 * - Angle Conversion: Conversion between degrees and radians.
 *
 * - Vec2 Operations: 2D vector mathematics including construction,
 *   arithmetic operators, dot product, distance calculations, length
 *   computation, normalization, and linear interpolation (lerp).
 *
 * - Vec3 Operations: 3D vector mathematics including all functions
 *   from Vec2, and cross product.
 *
 * - Utility Functions: Custom square root implementation using Newton's
 *   method and clamping function for value range restriction.
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

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

template<typename T>
T RG_DegToRad(T x) {
    return x * (RG_GetPI<T>() / (T)180.0);
}

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
T RG_ASin(T x) {
    if (x < (T)-1.0) x = (T)-1.0;
    if (x > (T)1.0) x = (T)1.0;
    return (T)asin((double)x);
}

template<typename T>
T RG_ACos(T x) {
    if (x < (T)-1.0) x = (T)-1.0;
    if (x > (T)1.0) x = (T)1.0;
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

#pragma endregion TRIGO

// ==============================================
// VEC2 CONSTRUCTORS AND OPERATORS
// ==============================================
#pragma region Vec2

// Constructors
Vec2::Vec2() : x(0.f), y(0.f) {}
Vec2::Vec2(float _x, float _y) : x(_x), y(_y) {}

// Operator overloads
Vec2 Vec2::operator*(float scalar) const {
    return Vec2(x * scalar, y * scalar);
}

Vec2 Vec2::operator+(const Vec2& rhs) const {
    return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator-(const Vec2& rhs) const {
    return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator/(float scalar) const {
    return Vec2(x / scalar, y / scalar);
}

Vec2& Vec2::operator+=(const Vec2& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

Vec2& Vec2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vec2& Vec2::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

// ==============================================
// VEC2 OPERATIONS
// ==============================================

Vec2 RG_Add(const Vec2& a, const Vec2& b) {
    return { a.x + b.x, a.y + b.y };
}

Vec2 RG_Subtract(const Vec2& a, const Vec2& b) {
    return { a.x - b.x, a.y - b.y };
}

Vec2 RG_Scale(const Vec2& v, float scalar) {
    return { v.x * scalar, v.y * scalar };
}

float RG_DotProduct(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
}

float RG_DistanceSquared(const Vec2& p1, const Vec2& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return dx * dx + dy * dy;
}

float RG_Distance(const Vec2& p1, const Vec2& p2) {
    return RG_Sqrt(RG_DistanceSquared(p1, p2));
}

float RG_Length(const Vec2& v) {
    return RG_Sqrt(v.x * v.x + v.y * v.y);
}

float RG_LengthSquared(const Vec2& v) {
    return v.x * v.x + v.y * v.y;
}

Vec2 RG_Normalize(const Vec2& v) {
    const float len = RG_Length(v);
    if (len > 0.f) {
        return { v.x / len, v.y / len };
    }
    return { 0.f, 0.f };
}
/**
 * @brief Linearly interpolates between two 2D vectors.
 *
 * @param a The starting vector.
 * @param b The ending vector.
 * @param t The interpolation parameter (0.0 returns a, 1.0 returns b).
 * @return Vec2 The interpolated vector at position t between a and b.
 */
Vec2 RG_Lerp(const Vec2& a, const Vec2& b, float t) {
    return RG_Add(a, RG_Scale(RG_Subtract(b, a), t));
}

#pragma endregion Vec2

// ==============================================
// VEC3 CONSTRUCTORS AND OPERATORS
// ==============================================
#pragma region Vec3

// Constructors
Vec3::Vec3() : x(0.f), y(0.f), z(0.f) {}
Vec3::Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

// Operator overloads
Vec3 Vec3::operator+(const Vec3& rhs) const {
    return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

Vec3 Vec3::operator-(const Vec3& rhs) const {
    return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vec3 Vec3::operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3 Vec3::operator/(float scalar) const {
    return Vec3(x / scalar, y / scalar, z / scalar);
}

Vec3& Vec3::operator+=(const Vec3& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

Vec3& Vec3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vec3& Vec3::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

// ==============================================
// VEC3 OPERATIONS
// ==============================================

Vec3 RG_Add(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 RG_Subtract(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 RG_Scale(const Vec3& v, float scalar) {
    return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

float RG_DotProduct(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 RG_CrossProduct(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,  // i 
        a.z * b.x - a.x * b.z,  // j 
        a.x * b.y - a.y * b.x   // k 
    );
}

float RG_Length(const Vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float RG_LengthSquared(const Vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float RG_DistanceSquared(const Vec3& p1, const Vec3& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    return dx * dx + dy * dy + dz * dz;
}

float RG_Distance(const Vec3& p1, const Vec3& p2) {
    return std::sqrt(RG_DistanceSquared(p1, p2));
}

Vec3 RG_Normalize(const Vec3& v) {
    float len = RG_Length(v);
    if (len > 0.0f) {
        return Vec3(v.x / len, v.y / len, v.z / len);
    }
    return Vec3(0.0f, 0.0f, 0.0f);
}

Vec3 RG_Lerp(const Vec3& a, const Vec3& b, float t) {
    return Vec3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

#pragma endregion Vec3

// ==============================================
// UTILITY FUNCTIONS
// ==============================================

/**
 * @brief Computes the square root using Newton's method.
 *
 * @param x The number to compute the square root of.
 * @return float The square root of x, or 0 if x is negative.
 */
float RG_Sqrt(float x) {
    if (x < 0) return 0;
    if (x == 0 || x == 1) return x;

    float precision = 0.00001f;
    float guess = x * 0.5f;

    for (int i = 0; i < 10; ++i) {
        float newGuess = 0.5f * (guess + x / guess);
        float abs = (newGuess - guess < 0) ? -(newGuess - guess) : (newGuess - guess);

        if (abs < precision) {
            return newGuess;
        }
        guess = newGuess;
    }
    return guess;
}

void RG_Clamp(float& val, float const& min, float const& max) {
    if (val > max) {
        val = max;
    }
    if (val < min) {
        val = min;
    }
}