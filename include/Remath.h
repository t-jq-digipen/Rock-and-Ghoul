#ifndef RG_MATH_H
#define RG_MATH_H

// --------------------------------------------------

/*
#include "RGVec2.h"
#include "RGMtx33.h"
#include "RGLineSegment2.h"

*/

#include "math.h"


#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus
	// note: need to figure out #define RG_API   __declspec(dllexport) for export.h

// TRIGO
// ==================================================
#pragma region TRIGO
	float RGDegToRad(float x);

	float RGRadToDeg(float x);

	float RGSin		(float x);

	float RGCos		(float x);

	float RGTan		(float x);

	float RGASin	(float x);

	float RGACos	(float x);

	float RGATan	(float x);

	// DEGREE VARIANTS
	#define RGSinDeg(x)     RGSin(AEDegToRad(x))
	#define RGCosDeg(x)     RGCos(AEDegToRad(x))
	#define RGTanDeg(x)     RGTan(AEDegToRad(x))
	#define RGASinDeg(x)    RGRadToDeg(AEASin(x))
	#define RGACosDeg(x)    RGRadToDeg(AEACos(x))
	#define RGATanDeg(x)    RGRadToDeg(AEATan(x))

#pragma endregion TRIGO
// ==================================================

	


#ifdef __cplusplus 
}
#endif //__cplusplus

#endif // RG_MATH_H
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

