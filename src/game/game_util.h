#include "engine/engine.h"

#ifndef GAME_UTIL_H
#define GAME_UTIL_H

#define RESOLUTION_WIDTH 320
#define RESOLUTION_HEIGHT 180

#define RESOLUTION_VEC2_HALF ((Vec2) {RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2})

Vec2 MousePosResolution(void);
Vec2 RandomPositionInLevel(void);
Vec2 RandomVec2InRange(Vec2 center, f32 radius);

#endif