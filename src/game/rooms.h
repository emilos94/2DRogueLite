#include "entities/entity.h"

#ifndef ROOMS_H
#define ROOMS_H

enum TileType
{
    TileType_None,

    TileType_Grass,
    TileType_Wall,
    TileType_Door,
    
    TileType_COUNT
};
typedef u8 TileType;

#define ROOM_MAX_TILE_WIDTH 40
#define ROOM_MAX_TILE_HEIGHT 40
#define ROOM_MAX_TILE_COUNT 1600
#define TILE_PIXEL_SIZE 16

typedef struct
{
    u32 Id;
    u32 RoomLeft;
    u32 RoomRight;
    
    Vec2 Position;
    Vec2 Size;

    u32 TileCountX;
    u32 TileCountY;

    boolean IsLocked;
    boolean IsActive;
    u32 EnemyAliveCount;

    EntityId Doors[Direction_COUNT]; // 1 possible door in each direction
    u32 DoorCount;

    TileType Tiles[ROOM_MAX_TILE_COUNT];
} Room;

#define ROOMS_CAPACITY 128
Room* RoomById(u32 id);
Room* CreateNewRoom(void);
void RoomsOnEntityEvent(EntityId id, EntityEvent event);
TileType RoomGetTileAt(Room* room, Vec2 pos);


#endif