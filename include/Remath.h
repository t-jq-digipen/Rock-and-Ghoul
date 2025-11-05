/* ========================================================
 * @file        Remath.h
 * @author      Yasmin Tazi			| yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa	| t.yingclarissa@digipen.edu
 * @date        01 OCT 2025
 *
 * @brief
 *
 * @brief
 * Header file for Remath library. This library includes
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
 * Copyright � 2025 DigiPen, All rights reserved.
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

/**
 * @brief Returns the absolute value of a number.
 *
 * @param x The number to get the absolute value of.
 * @return float The absolute value of x.
 */
float RG_Abs(float x);

/**
 * @brief Returns the minimum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 * @return float The smaller of the two values.
 */
float RG_Min(float a, float b);

/**
 * @brief Returns the maximum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 * @return float The larger of the two values.
 */
float RG_Max(float a, float b);

/**
 * @brief Returns the floor of a value (largest integer <= x).
 *
 * @param x The value to floor.
 * @return float The floor of x.
 */
float RG_Floor(float x);

/**
 * @brief Computes base raised to the power of exponent.
 *
 * @param base The base value.
 * @param exponent The exponent value.
 * @return float base^exponent.
 */
float RG_Pow(float base, float exponent);

// Clamp a value between two values
void RG_Clamp(float& val, float const& min, float const& max);

// VECTOR OPERATIONS (OVERLOADED FOR BOTH VEC2 AND VEC3)
// ==================================================
#pragma region VectorOperations

/**
 * @brief Adds two 2D vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return Vec2 The sum of the two vectors (a.x + b.x, a.y + b.y).
 */
Vec2 RG_Add(const Vec2& a, const Vec2& b);

/**
 * @brief Adds two 3D vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return Vec3 The sum of the two vectors.
 */
Vec3 RG_Add(const Vec3& a, const Vec3& b);

/**
 * @brief Subtracts one 2D vector from another.
 *
 * @param a The vector to subtract from.
 * @param b The vector to subtract.
 * @return Vec2 The difference (a - b).
 */
Vec2 RG_Subtract(const Vec2& a, const Vec2& b);

/**
 * @brief Subtracts one 3D vector from another.
 *
 * @param a The vector to subtract from.
 * @param b The vector to subtract.
 * @return Vec3 The difference (a - b).
 */
Vec3 RG_Subtract(const Vec3& a, const Vec3& b);

/**
 * @brief Scales a 2D vector by a scalar value.
 *
 * @param v The vector to scale.
 * @param scalar The scaling factor.
 * @return Vec2 The scaled vector (v.x * scalar, v.y * scalar).
 */
Vec2 RG_Scale(const Vec2& v, float scalar);

/**
 * @brief Scales a 3D vector by a scalar value.
 *
 * @param v The vector to scale.
 * @param scalar The scaling factor.
 * @return Vec3 The scaled vector.
 */
Vec3 RG_Scale(const Vec3& v, float scalar);

/**
 * @brief Computes the dot product of two 2D vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return float The dot product (a.x * b.x + a.y * b.y).
 */
float RG_DotProduct(const Vec2& a, const Vec2& b);

/**
 * @brief Computes the dot product of two 3D vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return float The dot product.
 */
float RG_DotProduct(const Vec3& a, const Vec3& b);

/**
 * @brief Computes the squared distance between two 2D points.
 *
 *
 * @param p1 The first point.
 * @param p2 The second point.
 * @return float The squared distance between the points.
 */
float RG_DistanceSquared(const Vec2& p1, const Vec2& p2);

/**
 * @brief Computes the squared distance between two 3D points.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 * @return float The squared distance between the points.
 */
float RG_DistanceSquared(const Vec3& p1, const Vec3& p2);

/**
 * @brief Computes the Euclidean distance between two 2D points.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 * @return float The distance between the points.
 */
float RG_Distance(const Vec2& p1, const Vec2& p2);

/**
 * @brief Computes the Euclidean distance between two 3D points.
 *
 * @param p1 The first point.
 * @param p2 The second point.
 * @return float The distance between the points.
 */
float RG_Distance(const Vec3& p1, const Vec3& p2);

/**
 * @brief Normalizes a 2D vector to unit length.
 *
 * @param v The vector to normalize.
 * @return Vec2 The normalized vector with length 1, or zero vector if input length is 0.
 */
Vec2 RG_Normalize(const Vec2& v);

/**
 * @brief Normalizes a 3D vector to unit length.
 *
 * @param v The vector to normalize.
 * @return Vec3 The normalized vector with length 1, or zero vector if input length is 0.
 */
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

/**
 * @brief Computes the length (magnitude) of a 2D vector.
 *
 * @param v The vector to measure.
 * @return float The length of the vector.
 */
float RG_Length(const Vec2& v);

/**
 * @brief Computes the length (magnitude) of a 3D vector.
 *
 * @param v The vector to measure.
 * @return float The length of the vector.
 */
float RG_Length(const Vec3& v);

/**
 * @brief Computes the squared length of a 2D vector.
 *
 *
 * @param v The vector to measure.
 * @return float The squared length of the vector.
 */
float RG_LengthSquared(const Vec2& v);

/**
 * @brief Computes the squared length of a 3D vector.
 *
 *
 * @param v The vector to measure.
 * @return float The squared length of the vector.
 */
float RG_LengthSquared(const Vec3& v);

// ==============================================
// VEC3-ONLY OPERATIONS
// ==============================================

/**
 * @brief Computes the cross product of two 3D vectors.
 *
 * Returns a vector perpendicular to both input vectors.
 *
 * @param a The first vector.
 * @param b The second vector.
 * @return Vec3 The cross product vector (a � b).
 */
Vec3 RG_CrossProduct(const Vec3& a, const Vec3& b);

#pragma endregion VectorOperations


#endif // RG_MATH_H
