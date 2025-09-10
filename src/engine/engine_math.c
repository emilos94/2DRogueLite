#include "engine_math.h"

boolean F32Equals(f32 a, f32 b)
{
    return fabsf(a - b) < ENGINE_MATH_FLOAT_EPSILON;
}

f32 DegreeToRadians(f32 degrees)
{
    float radians = degrees * (ENGINE_MATH_PI / 180.0);
    return radians;
}

f32 RadiansToDegrees(f32 radians)
{
    f32 degrees = radians * (180.0 / ENGINE_MATH_PI);
    return degrees;
}

f32 Lerp(f32 a, f32 b, f32 x)
{
    f32 result = a + (b - a) * x;
    return result;
}

boolean F32Between(f32 value, f32 min, f32 max)
{
    boolean result = value >= min && value <= max;
    return result;
}

f32 F32Clamp(f32 a, f32 min, f32 max)
{
    if (a < min)
    {
        return min;
    }
    if (a > max)
    {
        return max;
    }

    return a;
}

// Rand
f32 RandF32Ratio(void)
{
    f32 result = (f32)rand() / (f32)RAND_MAX;
    return result;
}

f32 RandF32Between(f32 min, f32 max)
{
    f32 result = min + (max - min) * RandF32Ratio();
    return result;
}

u32 RandU32Between(u32 min, u32 max)
{
    u32 result = min + (max - min) * RandF32Ratio();
    return result;
}

Vec2 RandVec2In(Vec2 min, Vec2 max)
{
    Vec2 result = (Vec2){
        .x = RandF32Between(min.x, max.x),
        .y = RandF32Between(min.y, max.y),
    };

    return result;
}

// Accepts percentage between 0 and 1 and checks if a random ratio is <= it
boolean RandChance(f32 chance)
{
    f32 ratio = RandF32Ratio();
    boolean result = ratio <= chance;
    return result;
}

// Easings
f32 EaseOutQuint(f32 a)
{
    f32 result = 1.0 - powf(1.0 - a, 5.0);
    return result;
}

// Vec2
Vec2 Vec2Normalize(Vec2 a)
{
    f32 magnitude = Vec2Magnitude(a);
    if (F32Equals(magnitude, 0.0))
    {
        return a;
    }

    Vec2 result = {
        .x = a.x / magnitude,
        .y = a.y / magnitude
    };
    return result;
}

Vec2 Vec2Add(Vec2 a, Vec2 b)
{
    Vec2 result = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return result;
}

Vec2 Vec2Addf(Vec2 a, f32 value)
{
    Vec2 result = {
        .x = a.x + value,
        .y = a.y + value
    };
    return result;
}

Vec2 Vec2Sub(Vec2 a, Vec2 b)
{
    Vec2 result = {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
    return result;
}

Vec2 Vec2Mul(Vec2 a, Vec2 b)
{
    Vec2 result = {
        .x = a.x * b.x,
        .y = a.y * b.y
    };
    return result;
}

Vec2 Vec2Mulf(Vec2 a, f32 scalar)
{
    Vec2 result = {
        .x = a.x * scalar,
        .y = a.y * scalar
    };
    return result;
}

f32 Vec2Magnitude(Vec2 a)
{
    f32 magnitude = sqrtf(a.x * a.x + a.y * a.y);
    return magnitude;
}

Vec2 Vec2Direction(Vec2 a, Vec2 b)
{
    Vec2 result = Vec2Sub(b, a);
    result = Vec2Normalize(result);
    return result;
}

f32 Vec2Distance(Vec2 a, Vec2 b)
{
    f32 deltaX = b.x - a.x;
    f32 deltaY = b.y - a.y;
    f32 result = sqrtf(deltaX * deltaX + deltaY * deltaY);
    return result;
}

f32 Vec2Dot(Vec2 a, Vec2 b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

Vec2 Vec2Lerp(Vec2 a, Vec2 b, f32 progress)
{
    Vec2 result = {
        .x = Lerp(a.x, b.x, progress),
        .y = Lerp(a.y, b.y, progress)
    };

    return result;
}

Vec2 Vec2Clamp(Vec2 a, Vec2 min, Vec2 max)
{
    Vec2 result = {
        .x = F32Clamp(a.x, min.x, max.x),
        .y = F32Clamp(a.y, min.y, max.y)
    };
    return result;
}

// Mat4
Mat4 Mat4Identity(void)
{
    Mat4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    return result;
}

Mat4 Mat4Orthographic(f32 left, f32 right, f32 bottom, f32 top)
{
    f32 far = 1.0;
    f32 near = 0.0;

    Mat4 result = {
        2.0 / (right - left), 0, 0, -((right + left) / (right - left)),
        0, 2.0 / (top - bottom), 0, -((top + bottom) / (top - bottom)),
        0, 0, -2.0 / (far - near), -((far + near) / (far - near)),
        0, 0, 0, 1.0
    };

    return result;
}

Mat4 Mat4ColumnMajor(Mat4 mat)
{
    Mat4 result = {};

    result.m[0] = mat.m[0];
    result.m[4] = mat.m[1];
    result.m[8] = mat.m[2];
    result.m[12] = mat.m[3];

    result.m[1] = mat.m[4];
    result.m[5] = mat.m[5];
    result.m[9] = mat.m[6];
    result.m[13] = mat.m[7];
    
    result.m[2] = mat.m[8];
    result.m[6] = mat.m[9];
    result.m[10] = mat.m[10];
    result.m[14] = mat.m[11];

    result.m[3] = mat.m[12];
    result.m[7] = mat.m[13];
    result.m[11] = mat.m[14];
    result.m[15] = mat.m[15];

    return result;
}

// Box2D
AABBCollisionInfo AABBCollision(Vec2 minA, Vec2 maxA, Vec2 minB, Vec2 maxB)
{
    AABBCollisionInfo result = {};
    f32 epsilon = 0.01f;

    f32 overLapX = fminf(maxA.x, maxB.x) - fmaxf(minA.x, minB.x);
    f32 overLapY = fminf(maxA.y, maxB.y) - fmaxf(minA.y, minB.y);

    result.Colliding = overLapX > 0 && overLapY > 0;

    if (!result.Colliding)
    {
        return result;
    }

    if (overLapX < overLapY)
    {
        f32 centerA = minA.x + maxA.x / 2.0;
        f32 centerB = minB.x + maxB.x / 2.0;

        if (centerA < centerB)
        {
            result.SeperationVector.x = -(overLapX + epsilon);
        }
        else
        {
            result.SeperationVector.x = overLapX + epsilon;
        }
    }
    else
    {
        f32 centerA = minA.y + maxA.y / 2.0;
        f32 centerB = minB.y + maxB.y / 2.0;

        if (centerA < centerB)
        {
            result.SeperationVector.y = -(overLapY + epsilon);
        }
        else
        {
            result.SeperationVector.y = overLapY + epsilon;
        }
    }

    return result;
}
