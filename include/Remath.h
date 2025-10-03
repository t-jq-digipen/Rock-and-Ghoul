/* ========================================================
 * @file        Remath.h
 * @author      Yasmin Tazi			| yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa	| t.yingclarissa@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * fella never even document this ------------- FIXTHISSHIT
 * Math library declaration (plz help me type more words i tired its almsot 4am)
 * -------------------------------------- end of FIXTHISSHIT
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

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
// TRIGO FUNCTIONS (RADIANS)
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

	Vec2();
	Vec2(float _x, float _y);

	Vec2 operator*(float scalar) const;
	Vec2 operator+(const Vec2& rhs) const;
	Vec2 operator-(const Vec2& rhs) const;
	Vec2 operator/(float scalar) const;
	Vec2& operator+=(const Vec2& rhs);
	Vec2& operator-=(const Vec2& rhs);
	Vec2& operator*=(float scalar);
	Vec2& operator/=(float scalar);
};

#pragma endregion 2DVectors

// 3D VECTORS
// ==================================================
#pragma region 3DVectors

struct Vec3 {
	float x, y, z;

	// Constructors
	Vec3();
	Vec3(float _x, float _y, float _z);

	// Operator overloads
	Vec3 operator+(const Vec3& rhs) const;
	Vec3 operator-(const Vec3& rhs) const;
	Vec3 operator*(float scalar) const;
	Vec3 operator/(float scalar) const;
	Vec3& operator+=(const Vec3& rhs);
	Vec3& operator-=(const Vec3& rhs);
	Vec3& operator*=(float scalar);
	Vec3& operator/=(float scalar);

};

#pragma endregion 3DVectors

// UTILITY FUNCTIONS
// ==================================================
// Find the square root of a value
/**
 * @brief Computes the square root using Newton's method.
 *
 * @param x The number to compute the square root of.
 * @return float The square root of x, or 0 if x is negative.
 */
float RG_Sqrt(float x);
// Clamp a value between two values
void RG_Clamp(float& val, float const& min, float const& max);

// VECTOR OPERATIONS (OVERLOADED FOR BOTH VEC2 AND VEC3)
// ==================================================
#pragma region VectorOperations

Vec2 RG_Add(const Vec2& a, const Vec2& b);
Vec3 RG_Add(const Vec3& a, const Vec3& b);

Vec2 RG_Subtract(const Vec2& a, const Vec2& b);
Vec3 RG_Subtract(const Vec3& a, const Vec3& b);

Vec2 RG_Scale(const Vec2& v, float scalar);
Vec3 RG_Scale(const Vec3& v, float scalar);

float RG_DotProduct(const Vec2& a, const Vec2& b);
float RG_DotProduct(const Vec3& a, const Vec3& b);

float RG_DistanceSquared(const Vec2& p1, const Vec2& p2);
float RG_DistanceSquared(const Vec3& p1, const Vec3& p2);

// Distance
float RG_Distance(const Vec2& p1, const Vec2& p2);
float RG_Distance(const Vec3& p1, const Vec3& p2);

// Normalize
Vec2 RG_Normalize(const Vec2& v);
Vec3 RG_Normalize(const Vec3& v);

// Linear Interpolation
/**
 * @brief Linearly interpolates between two 2D vectors.
 *
 * @param a The starting vector.
 * @param b The ending vector.
 * @param t The interpolation parameter (0.0 returns a, 1.0 returns b).
 * @return Vec2 The interpolated vector at position t between a and b.
 */
Vec2 RG_Lerp(const Vec2& a, const Vec2& b, float t);
Vec3 RG_Lerp(const Vec3& a, const Vec3& b, float t);

// Length
float RG_Length(const Vec2& v);
float RG_Length(const Vec3& v);

// Length Squared
float RG_LengthSquared(const Vec2& v);
float RG_LengthSquared(const Vec3& v);

// ==============================================
// VEC3-ONLY OPERATIONS
// ==============================================

// Cross product
Vec3 RG_CrossProduct(const Vec3& a, const Vec3& b);

#pragma endregion VectorOperations


#endif // RG_MATH_H
