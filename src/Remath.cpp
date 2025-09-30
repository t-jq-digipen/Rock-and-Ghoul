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

#pragma endregion TRIGO

// ==============================================
// VEC2 OPERATIONS
// ==============================================
#pragma region Vec2

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

Vec2 Lerp(const Vec2& a, const Vec2& b, float t) {
    return Add(a, Scale(Subtract(b, a), t));
}

Vec2 Normalize(const Vec2 a) {
    const float len = std::sqrt(a.x * a.x + a.y * a.y);

    if (len > 0.f) {
        return { a.x / len, a.y / len };
    }

    return { 0.f, 0.f };
}

#pragma endregion Vec2

// ==============================================
// VEC3 OPERATIONS
// ==============================================
#pragma region Vec3

Vec3 Add(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 Subtract(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 Scale(const Vec3& v, float scalar) {
    return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

// Result is positive if vectors point in similar directions, negative if opposite
// Result is zero if vectors are perpendicular
float DotProduct(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Cross Product
Vec3 CrossProduct(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,  // i 
        a.z * b.x - a.x * b.z,  // j 
        a.x * b.y - a.y * b.x   // k 
    );
}

// Length: calculates the size of a vector
// This is the distance from the origin to the point
float Length(const Vec3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float LengthSquared(const Vec3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

// Distance Squared: calculates squared distance between two points
float DistanceSquared(const Vec3& p1, const Vec3& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    return dx * dx + dy * dy + dz * dz;
}

// Distance: calculates actual distance between two points
float Distance(const Vec3& p1, const Vec3& p2) {
    return std::sqrt(DistanceSquared(p1, p2));
}

// Normalize: converts a vector to unit length (length = 1) while keeping its direction
// This is crucial for direction vectors in physics, graphics, AI, etc.
Vec3 Normalize(const Vec3& v) {
    float len = Length(v);

    // Avoid division by zero for zero-length vectors
    if (len > 0.0f) {
        return Vec3(v.x / len, v.y / len, v.z / len);
    }

    // Return zero vector if input was zero vector
    return Vec3(0.0f, 0.0f, 0.0f);
}

// Lerp: Linear interpolation between two vectors
// t=0 returns a, t=1 returns b, t=0.5 returns midpoint
// Values of t outside [0,1] will extrapolate beyond the points
Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
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

void Clamp(float& val, float const& min, float const& max) {
    if (val > max) {
        val = max;
    }
    if (val < min) {
        val = min;
    }
}