#ifndef RG_MATH_H
#define RG_MATH_H

#include <cmath>

// ==============================================
// CONSTANTS
// ==============================================
#define RG_PI       3.14159265358979323846
#define RG_PI_2     1.57079632679489661923
#define RG_2PI      6.28318530717958647692

// TRIGO
// ==================================================
#pragma region TRIGO

// ==============================================
// HELPER FUNCTIONS
// ==============================================
template <typename T>
T RG_GetPI();

// ==============================================
// ANGLE CONVERSION
// ==============================================
template <typename T>
T RG_DegToRad(T x);

template <typename T>
T RG_RadToDeg(T x);

// ==============================================
// TRIGO FUNCTIONS (Radians)
// ==============================================
template<typename T>
T RG_Sin(T x);

template<typename T>
T RG_Cos(T x);

template<typename T>
T RG_Tan(T x);

template<typename T>
T RG_ASin(T x);

template<typename T>
T RG_ACos(T x);

template<typename T>
T RG_ATan(T x);

template<typename T>
T RG_ATan2(T y, T x);

// ==============================================
// DEGREE VARIANTS
// ==============================================
template<typename T>
T RG_SinDeg(T x);

template<typename T>
T RG_CosDeg(T x);

template<typename T>
T RG_TanDeg(T x);

template<typename T>
T RG_ASinDeg(T x);

template<typename T>
T RG_ACosDeg(T x);

template<typename T>
T RG_ATanDeg(T x);

#pragma endregion TRIGO

#pragma once

struct Colour3 {
	float r, g, b;
};

struct Colour4 {
	float r, g, b, a;
};

// 2D VECTORS
// ==================================================
#pragma region 2DVectors

struct Vec2 {
	float x, y;

	Vec2() : x(0.f), y(0.f) {}
	Vec2(float _x, float _y) : x(_x), y(_y) {}

	Vec2 operator*(float scalar) const {
		return Vec2(x * scalar, y * scalar);
	}
	Vec2 operator+(const Vec2& rhs) const {
		return Vec2(x + rhs.x, y + rhs.y);
	}
	Vec2 operator-(const Vec2& rhs) const {
		return Vec2(x - rhs.x, y - rhs.y);
	}
	Vec2 operator/(float scalar) const {
		return Vec2(x / scalar, y / scalar);
	}
};

// Calculate squared distance between 2 points
float RG_DistanceSquared(const Vec2& p1, const Vec2& p2);
// Calculate dot product between 2 vectors
float RG_DotProduct(const Vec2& a, const Vec2& b);
// Addition between 2 vectors
Vec2 Add(const Vec2& a, const Vec2& b);
// Subtraction between 2 vectors
Vec2 Subtract(const Vec2& a, const Vec2& b);
// Scaling a vector
Vec2 Scale(const Vec2& v, float scalar);
// Linear interpolation
Vec2 Lerp(const Vec2& a, const Vec2& b, float t);
// Unit Vector
Vec2 Normalize(const Vec2 a);

#pragma endregion 2DVectors

// 3D VECTORS
// ==================================================
#pragma region 3DVectors

struct Vec3 {
	float x, y, z;

	// Constructors
	Vec3() : x(0.f), y(0.f), z(0.f) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	// Operator overloads
	Vec3 operator+(const Vec3& rhs) const {
		return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vec3 operator-(const Vec3& rhs) const {
		return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	Vec3 operator*(float scalar) const {
		return Vec3(x * scalar, y * scalar, z * scalar);
	}

	Vec3 operator/(float scalar) const {
		return Vec3(x / scalar, y / scalar, z / scalar);
	}

	Vec3& operator+=(const Vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	Vec3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
};

// Addition between 2 vectors
Vec3 Add(const Vec3& a, const Vec3& b);

// Subtraction between 2 vectors
Vec3 Subtract(const Vec3& a, const Vec3& b);

// Scaling a vector
Vec3 Scale(const Vec3& v, float scalar);

// Calculate dot product between 2 vectors
float DotProduct(const Vec3& a, const Vec3& b);

// Calculate cross product between 2 vectors (returns perpendicular vector)
Vec3 CrossProduct(const Vec3& a, const Vec3& b);

// Calculate the length of a vector
float Length(const Vec3& v);

// Calculate squared length
float LengthSquared(const Vec3& v);

// Calculate squared distance between 2 points
float DistanceSquared(const Vec3& p1, const Vec3& p2);

// Calculate actual distance between 2 points
float Distance(const Vec3& p1, const Vec3& p2);

// Normalize vector to unit length
Vec3 Normalize(const Vec3& v);

// Linear interpolation between two vectors
Vec3 Lerp(const Vec3& a, const Vec3& b, float t);

#pragma endregion 3DVectors

// ==================================================

// Find the square root of a value
float Sqrt(float x);

// Clamp a value between two values
void Clamp(float& val, float const& min, float const& max);

#endif // RG_MATH_H