#ifndef RG_MATH_H
#define RG_MATH_H
// --------------------------------------------------

/*
#include "RGVec2.h"
#include "RGMtx33.h"
#include "RGLineSegment2.h"

*/

#include <cmath>
// ==============================================
// CONSTANTS
// ==============================================
#define RG_PI       3.14159265358979323846
#define RG_PI_2     1.57079632679489661923
#define RG_2PI      6.28318530717958647692

//#ifdef __cplusplus
//extern "C"
//{
//#endif //__cplusplus
//	// note: need to figure out #define RG_API   __declspec(dllexport) for export.h

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
// ==================================================

	

//#ifdef __cplusplus 
//}
//#endif //__cplusplus

#endif // RG_MATH_H
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
// ==================================================

#pragma endregion 2DVectors

// 3D VECTORS
// ==================================================
struct Vec3 {
	float x, y, z;
};

// ==================================================

// Find the square root of a value
float Sqrt(float x);
// Clamp a value between two values
void Clamp(float& val, float const& min, float const& max);

