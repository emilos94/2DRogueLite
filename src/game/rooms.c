#include "rooms.h"

typedef struct RoomState
{
    Room Rooms[ROOMS_CAPACITY];
    u32 RoomCount;
    /* data */
} RoomState;
RoomState _RoomState = {};

Room* RoomById(s32 id)
{
    assert(id < ROOMS_CAPACITY && id > -1);

    return _RoomState.Rooms + id;
}

Room* CreateNewRoom(void)
{
    assert(_RoomState.RoomCount < ROOMS_CAPACITY - 1);

    Room* room = &_RoomState.Rooms[_RoomState.RoomCount++];
    memset(room, 0, sizeof(Room));
    
    room->IsActive = true;
    room->Id = _RoomState.RoomCount - 1;
    room->ConnectedRoomIds[0] = -1;
    room->ConnectedRoomIds[1] = -1;
    room->ConnectedRoomIds[2] = -1;
    room->ConnectedRoomIds[3] = -1;

    return room;
}

void _RoomsOnEntityDestroyed(Entity* entity)
{
    if (entity->Kind != EntityKind_Enemy)
    {
        return;
    }

    for (s32 i = 0; i < ROOMS_CAPACITY; i++)
    {
        Room* room = _RoomState.Rooms + i;
        if (!room->IsActive)
        {
            continue;
        }

        if (room->Id == entity->RoomId)
        {
            room->EnemyAliveCount--;
            if (room->EnemyAliveCount == 0)
            {
                printf("Cleared room!\n");

                for (s32 i = 0; i < room->GateCount; i++)
                {
                    EntityId gateId = room->Gates[i];
                    EntityQueueDestroy(gateId);
                }
            }
        }
    }
}

void _RoomsOnEntityCreated(Entity* entity)
{
    if (entity->Kind != EntityKind_Enemy)
    {
        return;
    }
    
    for (s32 i = 0; i < ROOMS_CAPACITY; i++)
    {
        Room* room = _RoomState.Rooms + i;
        if (!room->IsActive)
        {
            continue;
        }
        
        boolean inRoom = AABBCollision(room->Position, Vec2Add(room->Position, room->Size), entity->Position, Vec2Add(entity->Position, entity->Size)).Colliding;
        if (inRoom)
        {
            room->EnemyAliveCount++;
            entity->RoomId = room->Id;
        }
    }
}

void RoomsOnEntityEvent(EntityId id, EntityEvent event)
{
    Entity* entity = EntityById(id);
    if (!entity)
    {
        return;
    }

    switch (event)
    {
    case EntityEvent_Destroyed:
        _RoomsOnEntityDestroyed(entity);
    break;
    case EntityEvent_Created:
        _RoomsOnEntityCreated(entity);
    break; 
    default:
        break;
    }
}

TileType TileTypeFromPixel(u8 r, u8 g, u8 b)
{
    if (r == 122 && g == 72 && b == 65)
    {
        return TileType_Wall;
    }
    else if (r == 9 && g == 10 && b == 20)
    {
        // todo: spawn door entity and store reference in room
        return TileType_Door;
    }
    
    return TileType_Grass;
}

TileType RoomGetTileAt(Room* room, Vec2 pos)
{
    u32 x = (u32)(pos.x / TILE_PIXEL_SIZE);
    u32 y = (u32)(pos.y / TILE_PIXEL_SIZE);

    s32 index = y * room->TileCountX + x;
    if(index >= room->TileCountX * room->TileCountY)
    {
        return TileType_None;
    }

    return room->Tiles[index];
}

u32 RoomIdFromDirection(Direction direction)
{
    switch (direction)
    {
    case Direction_West: return 0;
    case Direction_East: return 1;
    case Direction_South: return 2;
    case Direction_North: return 3;
    default: return 0; // note should crash perhaps ?
    }
}

void ConnectRooms(Room* roomA, Room* roomB)
{
    assert(abs(roomA->MapX - roomB->MapX) <= 1 &&
           abs(roomA->MapY - roomB->MapY) <= 1);

    assert(roomA != roomB);

    // room a to the left | a <-> b
    if (roomA->MapX < roomB->MapX)
    {
        roomA->ConnectedRoomIds[RoomIdFromDirection(Direction_East)] = roomB->Id;
        roomB->ConnectedRoomIds[RoomIdFromDirection(Direction_West)] = roomA->Id;
    }
    // room a to the right | b <-> a
    else if (roomA->MapX > roomB->MapX)
    {
        roomA->ConnectedRoomIds[RoomIdFromDirection(Direction_West)] = roomB->Id;
        roomB->ConnectedRoomIds[RoomIdFromDirection(Direction_East)] = roomA->Id;
    }
    // room a below
    else if (roomA->MapY < roomB->MapY)
    {
        roomA->ConnectedRoomIds[RoomIdFromDirection(Direction_North)] = roomB->Id;
        roomB->ConnectedRoomIds[RoomIdFromDirection(Direction_South)] = roomA->Id;
    }
    // room a above
    else if (roomA->MapY > roomB->MapY)
    {
        roomA->ConnectedRoomIds[RoomIdFromDirection(Direction_South)] = roomB->Id;
        roomB->ConnectedRoomIds[RoomIdFromDirection(Direction_North)] = roomA->Id;
    }
}
