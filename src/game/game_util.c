#include "game_util.h"

Vec2 RandomPositionInLevel(void)
{
    Vec2 result = RandVec2In(
        (Vec2){ 8, 8 },
        (Vec2){ RESOLUTION_WIDTH - 8, RESOLUTION_HEIGHT - 16 }
    );

    return result;
}

Vec2 RandomVec2InRange(Vec2 center, f32 radius)
{
    Vec2 result = {
        .x = RandF32Between(center.x - radius, center.x + radius),
        .y = RandF32Between(center.y - radius, center.y + radius)
    };

    return result;
}

Vec2 MousePosResolution(void)
{
    Vec2 MousePos = MousePosition();
    Vec2 result = (Vec2) {
        .x = MousePos.x / WindowWidth() * RESOLUTION_WIDTH,
        .y = (1.0 - (MousePos.y / WindowHeight())) * RESOLUTION_HEIGHT
    };
    return result;
}
