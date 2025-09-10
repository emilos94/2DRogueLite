#include "game.h"
#include "entities/entity.h"
#include "entities/effects.h"
#include "rooms.h"

#include <pthread.h>

#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

// Constants
#define RESOLUTION_WIDTH 320
#define RESOLUTION_HEIGHT 180
#define RESOLUTION_VEC2_HALF ((Vec2) {RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2})

#define COLOR_WHITE ((Vec3){1,1,1})

// :util
Vec2 MousePosResolution(void);
Vec2 RandomPositionInLevel(void);
Vec2 RandomVec2InRange(Vec2 center, f32 radius);
Vec2 MousePosWorld(void);

#define DEBUG

#ifdef DEBUG
typedef struct DebugState
{
    boolean RenderCollisionShapes;
    boolean RenderMiniMap;
} DebugState;
DebugState debugState = {0};

#endif

enum GameMode
{
    GameMode_Playing,
    GameMode_GameOver
};
typedef u32 GameMode;

enum ZLayer 
{
    ZLayer_Ground,
    ZLayer_Tiles,
    ZLayer_Entity,
    ZLayer_UI0,
    ZLayer_UI1
};

// :map
typedef struct Map
{
    u32 RoomCountX, RoomCountY;
    s32* RoomIds;
    s32 StartingRoomId;
} Map;

typedef struct GameState 
{
    Texture Texture;
    boolean IsRunning;

    EntityId PlayerId;

    f32 ElapsedTime;

    SoundSource TestSound;

    GameMode GameMode;

    u32 CurrentRoomId;
    Room RoomTest;

    Vec2 CameraPosition;
    Vec2 CameraTargetPosition;

    Map map;
} GameState;
GameState gameState = {};

void* LoadResourcesBackground(void*);

void EntitiesUpdate(f32 delta);

// entities :entity
Entity* EntitySlimeCreate(Vec2 position);
Entity* EntityGateCreate(Vec2 position);

// :room
Room* GenerateNewRoom(void);
void RoomLoadTiles(Room* destination, const char* path);
void SpawnRoomEntities(Room* room);
void DrawRoomTiles(Room* room);
void SwitchToRoom(Direction directionFrom, Room* room);

// :map
Map GenerateMap(u32 roomCountX, u32 roomCountY, u32 roomsToPlace);
void AttemptRoomConnect(Map* map, Room* room, s32 mapX, s32 mapY);

void InitNewGame(void);
void GameOver(void);

#endif