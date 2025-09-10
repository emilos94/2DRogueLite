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
#define ROOM_GATE_CAPACITY 8
#define ROOM_DOORWAY_CAPACITY 8

typedef struct
{
    u32 Id;
    u32 RoomLeft;
    u32 RoomRight;
    
    Vec2 Position;
    Vec2 Size;

    u32 TileCountX;
    u32 TileCountY;

    u32 MapX, MapY;

    boolean IsLocked;
    boolean IsActive;
    boolean IsVisited;
    u32 EnemyAliveCount;

    s32 ConnectedRoomIds[4];

    EntityId Gates[ROOM_GATE_CAPACITY];
    u32 GateCount;

    TileType Tiles[ROOM_MAX_TILE_COUNT];
} Room;

#define ROOMS_CAPACITY 128
Room* RoomById(s32 id);
Room* CreateNewRoom(void);
void RoomsOnEntityEvent(EntityId id, EntityEvent event);
TileType RoomGetTileAt(Room* room, Vec2 pos);
void ConnectRooms(Room* roomA, Room* roomB);
u32 RoomIdFromDirection(Direction direction);

#endif