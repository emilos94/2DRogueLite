#include "game.h"
#include "entity.h"
#include "engine/engine.h"

#include <pthread.h>

#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#define DEBUG

#ifdef DEBUG
typedef struct DebugState
{
    boolean RenderCollisionShapes;
} DebugState;
DebugState debugState = {0};

#endif

typedef struct GameState 
{
    Texture Texture;
    boolean IsRunning;

    Texture* GrassTexture;
    Texture* WallsTexture;

    EntityId PlayerId;

    f32 ElapsedTime;

    SoundSource TestSound;

} GameState;
GameState gameState = {};

#define RESOLUTION_WIDTH 320
#define RESOLUTION_HEIGHT 180

Vec2 MousePosResolution(void);

void* LoadResourcesBackground(void*);

#endif