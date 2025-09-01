#include "game_internal.h"

void* LoadResourcesBackground(void* arguments)
{
    SoundInit();

    GameState* gameState = (GameState*)arguments;
    gameState->TestSound = SoundLoad("res/sounds/strike_blade_medium_003.wav");

    printf("Thread done, returning!\n");
    return NULL;
}

void EntitiesUpdate(f32 delta)
{
    Entity* entities = GetEntities(); 
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active))
        {
            continue;
        }

        entity->RenderScale.x = Lerp(entity->RenderScale.x, 1, 0.2);
        entity->RenderScale.y = Lerp(entity->RenderScale.y, 1, 0.2);

        if (entity->FlashTimer > 0)
        {
            entity->FlashTimer -= delta;
            f32 progress = 1.0 - (entity->FlashTimer / entity->FlashTime);
            entity->FlashStrength = 1.0 - EaseOutQuint(progress);
            if (entity->FlashTimer < 0)
            {
                entity->FlashTimer = 0;
                entity->FlashStrength = 0;
            }
        }

        if (entity->KnockBackAmount > 0)
        {
            entity->Position = Vec2Add(entity->Position, Vec2Mulf(entity->KnockBackDirection, entity->KnockBackAmount));
            entity->KnockBackAmount -= delta * 10;
            if (entity->KnockBackAmount < 0)
            {
                entity->KnockBackAmount = 0;
            }
        }

        if (entity->Flags & EntityFlag_HasHealth)
        {
            if (entity->Health <= 0)
            {
                entity->Health = 0;
                EntityQueueDestroy(entity->Id);
                if (entity->Id.Index == gameState.PlayerId.Index)
                {
                    GameOver();
                }
            }
        }

        if (entity->CustomCallback)
        {
            (*entity->CustomCallback)(entity, delta);
        }
    }
}

void InitNewGame(void)
{
    // :player
    Entity* player = EntityCreate(EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Animation | EntityFlag_RenderShadow | EntityFlag_HasHealth);
    player->Texture = GetTexture("player.png");
    player->Position = (Vec2){100, 100};
    player->BoundingBox = (BoundingBox){ .Offset = {5, 0}, .Size = {6, 6} };
    player->Animation.Data = GetAnimation(AnimationId_PlayerIdle);
    player->Animation.Timer = 0;
    player->AnimationId = AnimationId_PlayerIdle;
    player->WeaponAnchor = (Vec2) { 11, 7 };
    player->WeaponSwipe = 0.67;
    player->WeaponExtraRotation = 0;
    player->SwingSpeed = 0.2;
    player->Kind = EntityKind_Player;
    player->Health = 5;
    player->AttackCooldown = 0.5;
    player->AttackTimer = 0.0;

    gameState.PlayerId = player->Id;

    Vec2 position = RandomPositionInLevel();
    EntitySlimeCreate(position);
    
    position = RandomPositionInLevel();
    EntitySlimeCreate(position);

    position = RandomPositionInLevel();
    EntitySlimeCreate(position);

    gameState.GameMode = GameMode_Playing;
}

void GameOver(void)
{
    gameState.GameMode = GameMode_GameOver;
    memset(GetEntities(), 0, sizeof(Entity) * ENTITY_CAPACITY);
}

// :entity :start

// :entity :callbacks
void DestroyWhenKnockbackIsDown(Entity* entity, f32 delta)
{
    assert(entity);

    if (entity->KnockBackAmount <= 0)
    {
        EntityQueueDestroy(entity->Id);
    }
}

void SpawnPoof(Entity* entity, f32 delta)
{
    assert(entity);

    EffectCreatePoof(EntityCenterPos(entity));
}

void SpawnCorpse(Entity* entity, f32 delta)
{
    assert(entity);

    Entity* corpse = EntityCreate(EntityFlag_Render);
    corpse->Position = entity->Position;
    corpse->Texture = entity->Texture;
    corpse->KnockBackAmount = 3.5;
    corpse->KnockBackDirection = entity->KnockBackDirection;
    corpse->CustomCallback = DestroyWhenKnockbackIsDown;
    corpse->OnEntityDestroy = SpawnPoof;
}

// :slime
void SlimeCallback(Entity* slime, f32 delta)
{
    assert(slime);
    if (!slime->IsJumping)
    {   
        slime->JumpTimer += delta;
        if (slime->JumpTimer >= slime->JumpCooldown)
        {
            slime->JumpTimer = 0;
            slime->IsJumping = true;
            slime->RenderScale.x = 0.7;
            slime->RenderScale.y = 1.5;
            slime->JumpDriver = 0;

            slime->JumpStartPos = slime->Position;
            slime->JumpTarget = RandomVec2InRange(slime->Position, 40);
            while(slime->JumpTarget.x <= 8 || slime->JumpTarget.x >= RESOLUTION_WIDTH - 16 || slime->JumpTarget.y <= 8 || slime->JumpTarget.y >= RESOLUTION_HEIGHT - 16)
            {
                slime->JumpTarget = RandomVec2InRange(slime->Position, 40);
            }
        }
    }
    else
    {
        slime->JumpDriver += delta;
        f32 progress = slime->JumpDriver / slime->JumpTime;
        slime->Position = Vec2Lerp(slime->JumpStartPos, slime->JumpTarget, progress);

        // parabular
        slime->RenderOffsetY = -4.0 * slime->JumpHeight * (progress * progress - progress);

        if (slime->JumpDriver >= slime->JumpTime)
        {
            slime->RenderOffsetY = 0;
            slime->JumpDriver = 0;
            slime->IsJumping = false;
            slime->RenderScale.x = 1.7;
            slime->RenderScale.y = 0.4;
            Vec2 centerPos = EntityCenterPos(slime);
            centerPos.y -= 5;
            EffectCreateGroundImpact(centerPos);

            f32 range = 20;
            Entity* result[10];
            u32 count = EntityQueryInRange(centerPos, range, slime->Id, &result[0], 10);
            for (u32 i = 0; i < count; i++)
            {
                if (result[i]->Kind == EntityKind_Player)
                {
                    EntityReceiveDamage(result[i], Vec2Direction(centerPos, result[i]->Position), 2.5, 1);
                    result[i]->FlashColor = (Vec3) { .x = 0.8, .y = 0.3, .z = 0.3 };
                }
            }
        }
    }
}

// :slime
Entity* EntitySlimeCreate(Vec2 position)
{
    Entity* slime = EntityCreate(EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Render | EntityFlag_HasHealth | EntityFlag_RenderShadow);
    slime->Texture = GetTexture("slime.png");
    slime->Position = position;
    slime->BoundingBox = (BoundingBox){ .Offset = {5, 0}, .Size = {6, 6} };
    slime->Size = (Vec2) {16, 16};
    slime->Kind = EntityKind_Enemy;
    slime->Health = 3;
    slime->OnEntityDestroy = SpawnCorpse;
    slime->CustomCallback = SlimeCallback;

    slime->JumpCooldown = 3.0;
    slime->JumpTime = 1.0;
    slime->JumpTimer = RandF32Between(0, 1);
    slime->JumpHeight = 20.0;

    return slime;
}

Entity* EntityGateCreate(Vec2 position)
{
    Entity* door = EntityCreate(EntityFlag_Render | EntityFlag_Solid);
    door->Texture = GetTexture("door.png");
    door->Position = position;
    door->BoundingBox.Size = (Vec2) { door->Texture->Width / 2, door->Texture->Height };
    door->BoundingBox.Offset = (Vec2) { door->Texture->Width / 2, 0 };
    door->Size = (Vec2) { door->Texture->Width, door->Texture->Height };
    door->BoundingBox.Size = door->Size;
    return door;
}

// :doorway
void EntityDoorwayCollisionCallback(Entity* doorway, Entity* other)
{
    assert(doorway);
    assert(other);

    if (other->Kind != EntityKind_Player)
    {
        return;
    }
    
    if (doorway->RoomSwitchData.ConnectedRoom == -1)
    {
        // Create new room
        Room* newRoom = CreateNewRoom();

        // Link room to door and appropriate doorways together
        doorway->RoomSwitchData.ConnectedRoom = newRoom->Id;

        // Link to appropriate doorway
        EntityId doorwayIdToLink = newRoom->Doors[DirectionOpposite(doorway->RoomSwitchData.Placement)];
        Entity* doorwayToLink = EntityById(doorwayIdToLink);
        
        assert(doorwayToLink);

        doorway->RoomSwitchData.ConnectedDoorwayId = doorwayToLink->Id;
        doorwayToLink->RoomSwitchData.ConnectedDoorwayId = doorway->Id;
        doorwayToLink->RoomSwitchData.ConnectedRoom = gameState.CurrentRoomId;

        SwitchToRoom(doorway, newRoom);
    }
    else
    {
        // Switch to connected room
        Room* connectedRoom = RoomById(doorway->RoomSwitchData.ConnectedRoom);
        assert(connectedRoom);

        SwitchToRoom(doorway, connectedRoom);
    }
}

Entity* EntityDoorwayCreate(Vec2 position)
{
    Entity* doorway = EntityCreate(EntityFlag_Render | EntityFlag_Collider);
    doorway->Texture = GetTexture("doorway.png");
    doorway->Position = position;
    doorway->Size = (Vec2) { doorway->Texture->Width, doorway->Texture->Height };
    doorway->BoundingBox.Size = doorway->Size;
    doorway->OnCollision = EntityDoorwayCollisionCallback;
    doorway->RoomSwitchData.ConnectedRoom = -1;
    return doorway;
}

// :entity :end


// :room :draw
void DrawRoomTiles(Room* room)
{
    assert(room);

    for (s32 y = room->TileCountY - 1; y >= 0; y--)
    for (s32 x = 0; x < room->TileCountX; x++)
    {
        TileType tile = room->Tiles[room->TileCountX * 2 * y + x];
        switch (tile)
        {
        case TileType_Grass:
            DrawQuad(
                (Vec2) {.x = x * TILE_PIXEL_SIZE, .y = y * TILE_PIXEL_SIZE },
                (Vec2) { TILE_PIXEL_SIZE, TILE_PIXEL_SIZE },
                (Vec3) { 0.2, 0.8, 0.4}
            );
        break;
        case TileType_Wall:
            DrawQuad(
                (Vec2) {.x = x * TILE_PIXEL_SIZE, .y = y * TILE_PIXEL_SIZE },
                (Vec2) { TILE_PIXEL_SIZE, TILE_PIXEL_SIZE },
                (Vec3) { 0.7, 0.4, 0.7}
            );
        break;
        case TileType_Door:
            DrawQuad(
                (Vec2) {.x = x * TILE_PIXEL_SIZE, .y = y * TILE_PIXEL_SIZE },
                (Vec2) { TILE_PIXEL_SIZE, TILE_PIXEL_SIZE },
                (Vec3) { 0.7, 0.7, 0.7}
            );
        break;
        default:
            break;
        }
    }
}

Room* GenerateNewRoom(void)
{
    Room* room = CreateNewRoom();
    RoomLoadTiles(room, "res/rooms/test_map01.png"); // choose rand room data
    return room;
}

void SpawnRoomEntities(Room* room)
{
    for (u32 y = 0; y < ROOM_MAX_TILE_HEIGHT; y++)
    for (u32 x = 0; x < ROOM_MAX_TILE_WIDTH; x++)
    {
        TileType tileType = room->Tiles[y * ROOM_MAX_TILE_WIDTH + x];
        if (tileType == TileType_Door)
        {
            Vec2 position = (Vec2) { x * TILE_PIXEL_SIZE, y * TILE_PIXEL_SIZE };
            Entity* door = EntityGateCreate(position);
            room->Doors[room->DoorCount++] = door->Id;

            Direction doorDirection = Direction_East;
            if (x == 0)
            {
                doorDirection = Direction_West;
            }
            else if (y == 0)
            {
                doorDirection = Direction_South;
            }
            else if (y == ROOM_MAX_TILE_HEIGHT - 1)
            {
                doorDirection = Direction_North;
            }
            
            Entity* doorway = EntityGateCreate(position);
            doorway->RoomSwitchData.Placement = doorDirection;
            room->Doors[doorDirection] = doorway->Id;
        }
    }

}

void SwitchToRoom(Entity* doorwayFrom, Room* room)
{
    // todo: add screen effect
    gameState.CurrentRoomId = room->Id;
    
    Entity* player = EntityById(gameState.PlayerId);
    assert(player);

    if (doorwayFrom->DoorDirection == Direction_West)
    {
        player->Position.x = doorwayFrom->Position.x + (TILE_PIXEL_SIZE + 5);
    }
    else if (doorwayFrom->DoorDirection == Direction_East)
    {
        player->Position.x = doorwayFrom->Position.x - (TILE_PIXEL_SIZE + 5);
    }
}

// :room :tiles :load
void RoomLoadTiles(Room* destination, const char* path)
{
    s32 channels = 0;
    u32 width, height;
    u8* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data)
    {
		printf("Failed to load room image '%s'\n", path);
		assert(false);
    }

    destination->TileCountX = width / 2;
    destination->TileCountY = height;

    // read tile map frame
    for (u32 y = 0; y < destination->TileCountY; y++)
    for (u32 x = 0; x < destination->TileCountX; x++)
    {
        u32 index = (y * width + x) * 4;
        u8 red = data[index];
        u8 green = data[index+1];
        u8 blue = data[index+2];
        TileType tileType = TileTypeFromPixel(red, green, blue);
        destination->Tiles[y * width + x] = tileType;
    }

    // read entity frame
    for (u32 y = 0; y < height; y++)
    for (u32 x = destination->TileCountX - 1; x < width; x++)
    {
        u32 index = (y * width + x) * 4;
        u8 alpha = data[index+3];
        
        if (alpha > 0)
        {
            u8 red = data[index];
            u8 green = data[index+1];
            u8 blue = data[index+2];
            TileType tileType = TileTypeFromPixel(red, green, blue);

            if (tileType == TileType_Door)
            {
                u32 doorWidth = 8;
                Vec2 position = (Vec2) { x * TILE_PIXEL_SIZE - doorWidth, y * TILE_PIXEL_SIZE };

                Direction doorDirection = Direction_East;
                if (x == 0)
                {
                    position.x += doorWidth * 2;
                    doorDirection = Direction_West;
                }
                else if (y == 0)
                {
                    doorDirection = Direction_South;
                }
                else if (y == ROOM_MAX_TILE_HEIGHT - 1)
                {
                    doorDirection = Direction_North;
                }

                Entity* gate = EntityGateCreate(position);              
                
                Entity* doorway = EntityDoorwayCreate(position);
                doorway->RoomSwitchData.Placement = doorDirection;
                destination->Doors[doorDirection] = doorway->Id;
            }
        }
    }

    destination->IsActive = true;
    destination->Size = (Vec2) { destination->TileCountX * TILE_PIXEL_SIZE, destination->TileCountY * TILE_PIXEL_SIZE };
    destination->Position = (Vec2) { 0, 0 };
    stbi_image_free(data);
}

// :util
Vec2 MousePosWorld(void)
{
    Vec2 mousePosRes = MousePosResolution();
    mousePosRes = Vec2Add(mousePosRes, gameState.CameraPosition);
    return mousePosRes;
}

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

// :util :end