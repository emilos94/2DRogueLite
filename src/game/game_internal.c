#include "game_internal.h"

Vec2 MousePosResolution(void)
{
    Vec2 MousePos = MousePosition();
    Vec2 result = (Vec2) {
        .x = MousePos.x / WindowWidth() * RESOLUTION_WIDTH,
        .y = (1.0 - (MousePos.y / WindowHeight())) * RESOLUTION_HEIGHT
    };
    return result;
}

void* LoadResourcesBackground(void* arguments)
{
    SoundInit();

    GameState* gameState = (GameState*)arguments;
    gameState->TestSound = SoundLoad("res/sounds/strike_blade_medium_003.wav");

    printf("Thread done, returning!\n");
    return NULL;
}
