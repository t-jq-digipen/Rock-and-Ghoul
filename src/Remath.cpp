/* ========================================================
 * @file        Remath.cpp
 * @author      Yasmin Tazi         | yasmin.t@digipen.edu
 * @co-author   Tan Ying Clarissa   | t.yingclarissa@digipen.edu
 * @date        06 NOV 2025
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
 * - Mat2 Operations: 2x2 matrix mathematics including construction,
 *   arithmetic operators, matrix multiplication, transformations
 *   (scale, rotation), transpose, determinant, and inverse.
 *
 * - Mat3 Operations: 3x3 matrix mathematics including all functions
 *   from Mat2, and 3D rotations.
 * 
 * - Utility Functions: Custom square root implementation using Newton's
 *   method, clamping function for value range restriction, absolute value,
 *   min/max, floor, and power functions.
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

Vec2 Vec2::operator*(const Vec2& rhs) const {
    return Vec2(x * rhs.x, y * rhs.y);
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
// 2D MATRIX CONSTRUCTORS AND OPERATORS
// ==============================================
#pragma region 2DMatrix

Mat2::Mat2() {
    for (int col{ 0 }; col < 2; col++) {
        for (int row{ 0 }; row < 2; row++) {
            m[col][row] = 0.0f;
        }
    }
}

Mat2::Mat2(float m00, float m01,
    float m10, float m11) {
    m[0][0] = m00; m[0][1] = m01;
    m[1][0] = m10; m[1][1] = m11;
}

Mat2 Mat2::Identity() {
    return Mat2(
        1.0f, 0.0f,
        0.0f, 1.0f
    );
}

Mat2 Mat2::Zero() {
    return Mat2(
        0.0f, 0.0f,
        0.0f, 0.0f
    );
}

Mat2 Mat2::Scale(float sx, float sy) {
    return Mat2(
        sx, 0.0f,
        0.0f, sy
    );
}

Mat2 Mat2::Rotation(float angleRad) {
    float c = RG_Cos(angleRad);
    float s = RG_Sin(angleRad);
    return Mat2(
        c, s,
        -s, c
    );
}

Mat2 Mat2::operator+(const Mat2& rhs) const {
    Mat2 result;
    for (int col{ 0 }; col < 2; col++) {
        for (int row{ 0 }; row < 2; row++) {
            result.m[col][row] = m[col][row] + rhs.m[col][row];
        }
    }
    return result;
}

Mat2 Mat2::operator-(const Mat2& rhs) const {
    Mat2 result;
    for (int col{ 0 }; col < 2; col++) {
        for (int row{ 0 }; row < 2; row++) {
            result.m[col][row] = m[col][row] - rhs.m[col][row];
        }
    }
    return result;
}

Mat2 Mat2::operator*(float scalar) const {
    Mat2 result;
    for (int col{ 0 }; col < 2; col++) {
        for (int row{ 0 }; row < 2; row++) {
            result.m[col][row] = m[col][row] * scalar;
        }
    }
    return result;
}

Mat2 Mat2::operator*(const Mat2& rhs) const {
    Mat2 result;
    for (int col{ 0 }; col < 2; col++) {
        for (int row{ 0 }; row < 2; row++) {
            result.m[col][row] =
                m[0][row] * rhs.m[col][0] +
                m[1][row] * rhs.m[col][1];
        }
    }
    return result;
}

Vec2 Mat2::operator*(const Vec2& v) const {
    return Vec2(
        m[0][0] * v.x + m[1][0] * v.y,
        m[0][1] * v.x + m[1][1] * v.y
    );
}

#pragma endregion 2DMatrix

// ==============================================
// 3D MATRIX CONSTRUCTORS AND OPERATORS
// ==============================================
#pragma region 3DMatrix

Mat3::Mat3() {
    for (int col{ 0 }; col < 3; col++) {
        for (int row{ 0 }; row < 3; row++) {
            m[col][row] = 0.0f;
        }
    }
}

Mat3::Mat3(float m00, float m01, float m02,
    float m10, float m11, float m12,
    float m20, float m21, float m22) {
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22;
}

Mat3 Mat3::Identity() {
    return Mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    );
}

Mat3 Mat3::Zero() {
    return Mat3(
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    );
}

Mat3 Mat3::Scale(float sx, float sy, float sz) {
    return Mat3(
        sx, 0.0f, 0.0f,
        0.0f, sy, 0.0f,
        0.0f, 0.0f, sz
    );
}

Mat3 Mat3::RotationX(float angleRad) {
    float c = RG_Cos(angleRad);
    float s = RG_Sin(angleRad);
    return Mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, c, s,
        0.0f, -s, c
    );
}

Mat3 Mat3::RotationY(float angleRad) {
    float c = RG_Cos(angleRad);
    float s = RG_Sin(angleRad);
    return Mat3(
        c, 0.0f, -s,
        0.0f, 1.0f, 0.0f,
        s, 0.0f, c
    );
}

Mat3 Mat3::RotationZ(float angleRad) {
    float c = RG_Cos(angleRad);
    float s = RG_Sin(angleRad);
    return Mat3(
        c, s, 0.0f,
        -s, c, 0.0f,
        0.0f, 0.0f, 1.0f
    );
}

Mat3 Mat3::operator+(const Mat3& rhs) const {
    Mat3 result;
    for (int col{ 0 }; col < 3; col++) {
        for (int row{ 0 }; row < 3; row++) {
            result.m[col][row] = m[col][row] + rhs.m[col][row];
        }
    }
    return result;
}

Mat3 Mat3::operator-(const Mat3& rhs) const {
    Mat3 result;
    for (int col{ 0 }; col < 3; col++) {
        for (int row{ 0 }; row < 3; row++) {
            result.m[col][row] = m[col][row] - rhs.m[col][row];
        }
    }
    return result;
}

Mat3 Mat3::operator*(float scalar) const {
    Mat3 result;
    for (int col{ 0 }; col < 3; col++) {
        for (int row{ 0 }; row < 3; row++) {
            result.m[col][row] = m[col][row] * scalar;
        }
    }
    return result;
}

Mat3 Mat3::operator*(const Mat3& rhs) const {
    Mat3 result;
    for (int col{ 0 }; col < 3; col++) {
        for (int row{ 0 }; row < 3; row++) {
            result.m[col][row] =
                m[0][row] * rhs.m[col][0] +
                m[1][row] * rhs.m[col][1] +
                m[2][row] * rhs.m[col][2];
        }
    }
    return result;
}

Vec3 Mat3::operator*(const Vec3& v) const {
    return Vec3(
        m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
        m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
        m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z
    );
}

#pragma endregion 3DMatrix

// ==============================================
// MATRIX OPERATIONS
// ==============================================
#pragma region MatrixOperations

Mat2 RG_Transpose(const Mat2& mat) {
    return Mat2(
        mat.m[0][0], mat.m[1][0],
        mat.m[0][1], mat.m[1][1]
    );
}

Mat3 RG_Transpose(const Mat3& mat) {
    return Mat3(
        mat.m[0][0], mat.m[1][0], mat.m[2][0],
        mat.m[0][1], mat.m[1][1], mat.m[2][1],
        mat.m[0][2], mat.m[1][2], mat.m[2][2]
    );
}

float RG_Determinant(const Mat2& mat) {
    return mat.m[0][0] * mat.m[1][1] - mat.m[1][0] * mat.m[0][1];
}

float RG_Determinant(const Mat3& mat) {
    float a = mat.m[0][0];
    float b = mat.m[1][0];
    float c = mat.m[2][0];

    float minor0 = mat.m[1][1] * mat.m[2][2] - mat.m[2][1] * mat.m[1][2];
    float minor1 = mat.m[0][1] * mat.m[2][2] - mat.m[2][1] * mat.m[0][2];
    float minor2 = mat.m[0][1] * mat.m[1][2] - mat.m[1][1] * mat.m[0][2];

    return a * minor0 - b * minor1 + c * minor2;
}

Mat2 RG_Inverse(const Mat2& mat) {
    float det = RG_Determinant(mat);

    if (RG_Abs(det) < 0.00001f) {
        return Mat2::Identity();
    }

    float invDet = 1.0f / det;

    return Mat2(
        mat.m[1][1] * invDet, -mat.m[0][1] * invDet,
        -mat.m[1][0] * invDet, mat.m[0][0] * invDet
    );
}

Mat3 RG_Inverse(const Mat3& mat) {
    float det = RG_Determinant(mat);

    if (RG_Abs(det) < 0.00001f) {
        return Mat3::Identity();
    }

    float invDet = 1.0f / det;

    Mat3 cofactor;

    // Row 0
    cofactor.m[0][0] = (mat.m[1][1] * mat.m[2][2] - mat.m[2][1] * mat.m[1][2]);
    cofactor.m[1][0] = -(mat.m[0][1] * mat.m[2][2] - mat.m[2][1] * mat.m[0][2]);
    cofactor.m[2][0] = (mat.m[0][1] * mat.m[1][2] - mat.m[1][1] * mat.m[0][2]);

    // Row 1
    cofactor.m[0][1] = -(mat.m[1][0] * mat.m[2][2] - mat.m[2][0] * mat.m[1][2]);
    cofactor.m[1][1] = (mat.m[0][0] * mat.m[2][2] - mat.m[2][0] * mat.m[0][2]);
    cofactor.m[2][1] = -(mat.m[0][0] * mat.m[1][2] - mat.m[1][0] * mat.m[0][2]);

    // Row 2
    cofactor.m[0][2] = (mat.m[1][0] * mat.m[2][1] - mat.m[2][0] * mat.m[1][1]);
    cofactor.m[1][2] = -(mat.m[0][0] * mat.m[2][1] - mat.m[2][0] * mat.m[0][1]);
    cofactor.m[2][2] = (mat.m[0][0] * mat.m[1][1] - mat.m[1][0] * mat.m[0][1]);

    Mat3 result;
    for (int col{ 0 }; col < 3; col++) {
        for (int row{ 0 }; row < 3; row++) {
            result.m[col][row] = cofactor.m[row][col] * invDet;
        }
    }

    return result;
}
#pragma endregion MatrixOperations

// ==============================================
// UTILITY FUNCTIONS
// ==============================================
#pragma region UtilityFunctions
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

/**
 * @brief Returns the absolute value of a number.
 *
 * @param x The number to get the absolute value of.
 * @return float The absolute value of x.
 */
float RG_Abs(float x) {
    return (x < 0.0f) ? -x : x;
}

/**
 * @brief Returns the minimum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 * @return float The smaller of the two values.
 */
float RG_Min(float a, float b) {
    return (a < b) ? a : b;
}

/**
 * @brief Returns the maximum of two values.
 *
 * @param a The first value.
 * @param b The second value.
 * @return float The larger of the two values.
 */
float RG_Max(float a, float b) {
    return (a > b) ? a : b;
}

/**
 * @brief Returns the floor of a value (largest integer <= x).
 *
 * @param x The value to floor.
 * @return float The floor of x.
 */
float RG_Floor(float x) {
    int intPart = static_cast<int>(x);
    // Handle negative numbers correctly
    if (x < 0.0f && x != static_cast<float>(intPart)) {
        return static_cast<float>(intPart - 1);
    }
    return static_cast<float>(intPart);
}

/**
 * @brief Computes base raised to the power of exponent.
 *
 * Uses natural logarithm and exponential for accurate results.
 *
 * @param base The base value.
 * @param exponent The exponent value.
 * @return float base^exponent.
 */
float RG_Pow(float base, float exponent) {
    // Handle special cases
    if (exponent == 0.0f) return 1.0f;
    if (exponent == 1.0f) return base;
    if (base == 0.0f) return 0.0f;
    if (base == 1.0f) return 1.0f;
    if (base < 0.0f && exponent != static_cast<int>(exponent)) {
        // Negative base with non-integer exponent is undefined in real numbers
        return 0.0f;
    }

    // For calculations like physics damping, we need to use built-in power
    // I'm sorry I really can't calculate it ;-;
    return std::pow(base, exponent);
}

#pragma endregion UtilityFunctions

