#include "engine_internal.h"
#include <math.h>

#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#define ENGINE_MATH_FLOAT_EPSILON 0.001
#define ENGINE_MATH_PI 3.1427

//utils
boolean F32Equals(f32 a, f32 b);
f32 DegreeToRadians(f32 degrees);
f32 RadiansToDegrees(f32 radians);
f32 Lerp(f32 a, f32 b, f32 x);
boolean F32Between(f32 value, f32 min, f32 max);
f32 F32Clamp(f32 value, f32 min, f32 max);

// Easings
f32 EaseOutQuint(f32 a);

// Vec2
typedef struct Vec2
{
    f32 x, y;
} Vec2;

Vec2 Vec2Normalize(Vec2 a);
Vec2 Vec2Add(Vec2 a, Vec2 b);
Vec2 Vec2Addf(Vec2 a, f32 value);
Vec2 Vec2Sub(Vec2 a, Vec2 b);
Vec2 Vec2Mul(Vec2 a, Vec2 b);
Vec2 Vec2Mulf(Vec2 a, f32 scalar);
f32 Vec2Magnitude(Vec2 a);
Vec2 Vec2Direction(Vec2 a, Vec2 b);
f32 Vec2Distance(Vec2 a, Vec2 b);
f32 Vec2Dot(Vec2 a, Vec2 b);
Vec2 Vec2Lerp(Vec2 a, Vec2 b, f32 progress);
Vec2 Vec2Clamp(Vec2 a, Vec2 min, Vec2 max);


// Vec3
typedef struct Vec3
{
    f32 x, y, z;
} Vec3;

// Mat4
typedef struct Mat4
{
    f32 m[16];
} Mat4;

Mat4 Mat4Identity(void);
Mat4 Mat4Orthographic(f32 left, f32 right, f32 bottom, f32 top);
Mat4 Mat4ColumnMajor(Mat4 mat);

// Box collisions
typedef struct AABBCollisionInfo
{
    Vec2 SeperationVector;
    boolean Colliding;
} AABBCollisionInfo;

AABBCollisionInfo AABBCollision(Vec2 minA, Vec2 maxA, Vec2 minB, Vec2 maxB);

// Rand
f32 RandF32Ratio(void);
f32 RandF32Between(f32 min, f32 max);
Vec2 RandVec2In(Vec2 min, Vec2 max);

#endif