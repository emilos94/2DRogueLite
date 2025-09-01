#include "rooms.h"

typedef struct RoomState
{
    Room Rooms[ROOMS_CAPACITY];
    u32 RoomCount;
    /* data */
} RoomState;
RoomState _RoomState = {};

Room* RoomById(u32 id)
{
    assert(id < ROOMS_CAPACITY);

    return _RoomState.Rooms + id;
}

Room* CreateNewRoom(void)
{
    assert(_RoomState.RoomCount < ROOMS_CAPACITY - 1);

    Room* room = &_RoomState.Rooms[_RoomState.RoomCount++];
    room->IsActive = true;
    room->Id = _RoomState.RoomCount - 1;

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

                for (s32 i = 0; i < room->DoorCount; i++)
                {
                    EntityId doorId = room->Doors[i];
                    Entity* door = EntityById(doorId);
                    if (door)
                    {
                        door->Flags &= EntityFlag_Render;
                    }
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
        
        boolean inRoom = AABBCollision(room->Position, Vec2Add(room->Position, room->Size), entity->Position, entity->Size).Colliding;
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
    u32 x = F32Clamp(pos.x / TILE_PIXEL_SIZE, 0, ROOM_MAX_TILE_WIDTH * TILE_PIXEL_SIZE);
    u32 y = F32Clamp(pos.y / TILE_PIXEL_SIZE, 0, ROOM_MAX_TILE_HEIGHT * TILE_PIXEL_SIZE);

    s32 index = y * ROOM_MAX_TILE_WIDTH + x;
    if(index >= ROOM_MAX_TILE_COUNT)
    {
        return TileType_None;
    }

    return room->Tiles[index];
}
