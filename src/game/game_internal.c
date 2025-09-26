#include "game_internal.h"

void* LoadSoundResourcesBackground(void* arguments)
{
    SoundInit();
    LoadSounds();

    printf("Thread done, returning!\n");
    return NULL;
}

// :entity :update
void EntitiesUpdate(f32 delta)
{
    Entity* entities = GetEntities(); 
    Room* room = RoomById(gameState.CurrentRoomId);

    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active))
        {
            continue;
        }

        if (entity->RoomId != gameState.CurrentRoomId)
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
            entity->Position = Vec2Clamp(
                Vec2Add(entity->Position, Vec2Mulf(entity->KnockBackDirection, entity->KnockBackAmount)),
                V2(16, 16),
                Vec2Sub(room->Size, V2(16, 16))
            );

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

        if (entity->InvulnerableTimer > 0)
        {
            entity->InvulnerableTimer -= delta;
            if (entity->InvulnerableTimer <= 0)
            {
                entity->InvulnerableTimer = 0;
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
    memset(GetEntities(), 0, sizeof(Entity) * ENTITY_CAPACITY);

    // :player
    Entity* player = EntityCreate(EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Animation | EntityFlag_RenderShadow | EntityFlag_HasHealth);
    player->Texture = GetTexture("player.png");
    player->Position = (Vec2){100, 100};
    player->BoundingBox = (BoundingBox){ .Offset = {5, 0}, .Size = {6, 6} };
    player->Size = V2(6, 6);
    player->Animation.Data = GetAnimation(AnimationId_PlayerIdle);
    player->Animation.Timer = 0;
    player->AnimationId = AnimationId_PlayerIdle;
    player->WeaponAnchor = (Vec2) { 11, 7 };
    player->WeaponSwipe = 0.67;
    player->WeaponExtraRotation = 0;
    player->AttackSpeed = 0.2;
    player->Kind = EntityKind_Player;
    player->Health = 5;
    player->AttackCooldown = 0.5;
    player->AttackTimer = 0.0;
    player->RoomId = gameState.CurrentRoomId;
    player->ShadowSize = ShadowSize_Small;

    gameState.PlayerId = player->Id;

    for (s32 i = 0; i < 5; i++)
    {
        Vec2 position = RandomPositionInLevel();
        Entity* slime = EntitySlimeCreate(position);
        slime->RoomId = gameState.CurrentRoomId;
    }

    gameState.GameMode = GameMode_Playing;
}

void GameOver(void)
{
    gameState.GameMode = GameMode_GameOver;
    gameState.CoinCount = 0;
    gameState.GameOverhintTextAlpha = 1.0;
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
    corpse->RenderScale = V2(1.5, 0.7);
    EntityFlash(corpse, COLOR_WHITE, 1.0);
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
            Room* currentRoom = RoomById(gameState.CurrentRoomId);

            slime->JumpTarget = RandomVec2InRange(slime->Position, 40);
            while(slime->JumpTarget.x <= 8 || slime->JumpTarget.x >= currentRoom->TileCountX * TILE_PIXEL_SIZE - 16 || slime->JumpTarget.y <= 8 || slime->JumpTarget.y >= currentRoom->TileCountY * TILE_PIXEL_SIZE - 16)
            {
                slime->JumpTarget = RandomVec2InRange(slime->Position, 40);
            }

            EntityJumpTo(slime, slime->JumpTarget, slime->JumpHeight);
        }
    }
}

// :slime
void OnSlimeLandCallback(Entity* slime, f32 delta)
{
    Vec2 centerPos = EntityCenterPos(slime);
    centerPos.y -= 5;
    Entity* groundImpact = EffectCreateGroundImpact(centerPos);
    groundImpact->RoomId = slime->RoomId;

    Bullet* left = CreateBullet(slime->Position, V2(-BULLET_DEFAULT_SPEED, 0));
    Bullet* right = CreateBullet(slime->Position, V2(BULLET_DEFAULT_SPEED, 0));
    Bullet* up = CreateBullet(slime->Position, V2(0, BULLET_DEFAULT_SPEED));
    Bullet* down = CreateBullet(slime->Position, V2(0, -BULLET_DEFAULT_SPEED));

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

// :entity
void EntityReceiveDamage(Entity* entity, Vec2 direction, f32 knockBackAmount, f32 damage)
{
    assert(entity);

    if (entity->InvulnerableTimer > 0)
    {
        return;
    }

    if (entity->Id.Index == gameState.PlayerId.Index)
    {
        entity->InvulnerableTimer = PLAYER_INVULNERABLE_ON_DMG_TIME;
    }

    entity->RenderScale.x = 1.7;
    entity->RenderScale.y = 0.4;
    EntityFlash(entity, (Vec3){1.0, 1.0, 1.0}, 1);
    entity->KnockBackAmount = knockBackAmount;
    entity->KnockBackDirection = direction;
    entity->Health -= damage;

    if (entity->OnReceiveDamageSound)
    {
        SoundSource* sound = GetSound(entity->OnReceiveDamageSound);
        SoundPlay(sound);
    }
}

// :entity
void EntityJumpTo(Entity* entity, Vec2 position, f32 jumpHeight)
{
    assert(entity);
    assert(entity->Flags & EntityFlag_Jump);

    entity->JumpTimer = 0;
    entity->IsJumping = true;
    entity->RenderScale.x = 0.7;
    entity->RenderScale.y = 1.5;
    entity->JumpDriver = 0;
    entity->JumpStartPos = entity->Position;
    entity->JumpTarget = position;
    entity->JumpHeight = jumpHeight;
}

// :entity :event
void GenericOnEntityEvent(EntityId entityId, EntityEvent event)
{
    Entity* entity = EntityById(entityId);

    if (!entity)
    {
        return;
    }

    if (event == EntityEvent_Created)
    {
        entity->CreatedTime = gameState.ElapsedTime;
    }

    if (event == EntityEvent_Destroyed && entity->Kind == EntityKind_Enemy)
    {
        for (s32 i = 0; i < 3; i++)
        {
            EntityCoinCreate(entity->Position);
        }
    }
}

// :slime
Entity* EntitySlimeCreate(Vec2 position)
{
    Entity* slime = EntityCreate(
        EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Render | 
        EntityFlag_HasHealth | EntityFlag_RenderShadow | EntityFlag_Jump
    );

    slime->Texture = GetTexture("slime.png");
    slime->Position = position;
    slime->BoundingBox = (BoundingBox){ .Offset = {5, 0}, .Size = {6, 6} };
    slime->Size = (Vec2) {16, 16};
    slime->Kind = EntityKind_Enemy;
    slime->Health = 3;
    slime->OnEntityDestroy = SpawnCorpse;
    slime->CustomCallback = SlimeCallback;
    slime->OnLandFromJumpCallback = OnSlimeLandCallback;

    slime->JumpCooldown = 3.0;
    slime->JumpTime = 1.0;
    slime->JumpTimer = RandF32Between(0, 1);
    slime->JumpHeight = 20.0;

    slime->OnReceiveDamageSound = "receive_dmg.wav";
    slime->ShadowSize = ShadowSize_Small;

    return slime;
}

// :skeleton
void SkeletonCallback(Entity* skeleton, f32 delta)
{
    Entity* player = EntityById(gameState.PlayerId);
    if (Vec2Distance(player->Position, skeleton->Position) <= 100)
    {
        Vec2 dirToPlayer = Vec2Direction(skeleton->Position, player->Position);
        skeleton->Velocity = Vec2Mulf(dirToPlayer, 30);
        skeleton->IsPrimaryActionActive = true;
    }
    else
    {
        skeleton->IsPrimaryActionActive = false;
    }

    if (skeleton->PrimaryActionTimer > 0 && skeleton->IsPrimaryActionActive)
    {
        skeleton->PrimaryActionTimer -= delta;
        printf("%.2f\n", skeleton->PrimaryActionTimer);

        if (skeleton->PrimaryActionTimer <= 0)
        {
            printf("Skeleton primary action!\n");
            Vec2 dirToPlayer = Vec2Direction(skeleton->Position, player->Position);
            Bullet* bullet = CreateBullet(skeleton->Position, Vec2Mulf(dirToPlayer, BULLET_DEFAULT_SPEED));
        
            skeleton->RenderScale.x = 1.5;
            skeleton->RenderScale.y = 0.7;
        
            skeleton->PrimaryActionTimer = 5;
        }
    }
}


// :skeleton
Entity* EntitySkeletonCreate(Vec2 position)
{
    Entity* skeleton = EntityCreate(
        EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Render | 
        EntityFlag_HasHealth | EntityFlag_RenderShadow | EntityFlag_FlipXOnMove
    );

    skeleton->Texture = GetTexture("skeleton.png");
    skeleton->Position = position;
    skeleton->BoundingBox = (BoundingBox){ .Offset = {5, 0}, .Size = {12, 20} };
    skeleton->Size = (Vec2) {12, 25};
    skeleton->Kind = EntityKind_Enemy;
    skeleton->Health = 3;
    skeleton->OnEntityDestroy = SpawnCorpse;
    skeleton->CustomCallback = SkeletonCallback;

    skeleton->OnReceiveDamageSound = "receive_dmg.wav";
    skeleton->ShadowSize = ShadowSize_Small;
    skeleton->PrimaryActionTimer = 1;

    return skeleton;
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

// :coin
void CoinCallback(Entity* coin, f32 delta)
{
    if (!coin->IsJumping)
    {
        coin->RenderOffsetY = 2 + sin(gameState.ElapsedTime) * 2.0 - 1.0 * 4;
    }
}

// :coin
void CoinCollisionCallback(Entity* coin, Entity* other)
{
    assert(coin);
    assert(other);

    if (other->Kind == EntityKind_Player)
    {
        EntityQueueDestroy(coin->Id);
        gameState.CoinCount++;
        SoundPlay(GetSound("small_pickup.wav"));
    }
}

// :coin
Entity* EntityCoinCreate(Vec2 position)
{
    Entity* coin = EntityCreate(
        EntityFlag_Render | EntityFlag_Collectible | EntityFlag_Jump | 
        EntityFlag_RenderShadow | EntityFlag_Collider
    );
    
    coin->Kind = EntityKind_Loot;
    coin->Texture = GetTexture("coin_8x8.png");
    coin->Position = position;
    coin->BoundingBox = (BoundingBox) { .Offset = {0, 0}, .Size = {8, 8} };
    
    coin->JumpTime = 1.0;
    Vec2 jumpTarget = RandomVec2InRange(position, 30);
    EntityJumpTo(coin, jumpTarget, 10);

    coin->CustomCallback = CoinCallback;
    coin->OnCollision = CoinCollisionCallback;

    coin->ShadowSize = ShadowSize_Mini;
    coin->PickupRange = 30;

    return coin;
}

// :entity :end


// :room :draw
void DrawRoomTiles(Room* room)
{
    assert(room);

    Texture* wallTexture = GetTexture("wall_tile.png");

    for (s32 y = room->TileCountY - 1; y >= 0; y--)
    for (s32 x = 0; x < room->TileCountX; x++)
    {
        TileType tile = room->Tiles[room->TileCountX * y + x];
        switch (tile)
        {
        case TileType_Wall:
            QuadDrawCmd* cmdWall = DrawTexture(
                (Vec2) {.x = x * TILE_PIXEL_SIZE, .y = y * TILE_PIXEL_SIZE },
                wallTexture
            );
            cmdWall->ZLayer = ZLayer_Tiles;
        break;
        case TileType_Door:
            QuadDrawCmd* cmdDoor = DrawQuad(
                (Vec2) {.x = x * TILE_PIXEL_SIZE, .y = y * TILE_PIXEL_SIZE },
                (Vec2) { TILE_PIXEL_SIZE, TILE_PIXEL_SIZE },
                (Vec3) { 0.7, 0.7, 0.7}
            );
            cmdDoor->ZLayer = ZLayer_Tiles;
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
        }
    }

}

void SwitchToRoom(Direction directionFrom, Room* room)
{
    // todo: add screen effect
    gameState.CurrentRoomId = room->Id;
    
    Entity* player = EntityById(gameState.PlayerId);
    assert(player);

    if (!room->IsVisited)
    {
        for (s32 i = 0; i < 3; i++)
        {
            Vec2 pos = RandomPositionInLevel();
            Entity* slime = EntitySlimeCreate(pos);
            slime->RoomId = room->Id;
        }

        for (s32 i = 0; i < 3; i++)
        {
            Vec2 pos = RandomPositionInLevel();
            Entity* slime = EntitySkeletonCreate(pos);
            slime->RoomId = room->Id;
        }

        room->IsVisited = true;
    }

    if (directionFrom == Direction_East)
    {
        player->Position.x = room->TileCountX * TILE_PIXEL_SIZE - (TILE_PIXEL_SIZE * 2);
    }
    else if (directionFrom == Direction_West)
    {
        player->Position.x = TILE_PIXEL_SIZE + 5;
    }
    else if (directionFrom == Direction_North)
    {
        player->Position.y = room->TileCountY * TILE_PIXEL_SIZE - (TILE_PIXEL_SIZE + 5);
    }
    else if (directionFrom == Direction_South)
    {
        player->Position.y = TILE_PIXEL_SIZE + 5;
    }
    player->RoomId = room->Id;
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
        destination->Tiles[y * destination->TileCountX + x] = tileType;
    }

    // read entity frame
    for (u32 y = 0; y < height; y++)
    for (u32 x = destination->TileCountX; x < width; x++)
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
                Vec2 position = (Vec2) { (x - destination->TileCountX) * TILE_PIXEL_SIZE - doorWidth, y * TILE_PIXEL_SIZE };

                Direction doorDirection = Direction_East;
                if (x == destination->TileCountX)
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
                destination->Gates[destination->GateCount++] = gate->Id;          
            }
        }
    }

    destination->IsActive = true;
    destination->Size = (Vec2) { destination->TileCountX * TILE_PIXEL_SIZE, destination->TileCountY * TILE_PIXEL_SIZE };
    destination->Position = (Vec2) { 0, 0 };
    stbi_image_free(data);
}

// :maps
Map GenerateMap(u32 roomCountX, u32 roomCountY, u32 roomsToPlace)
{
    Map result = {
        .RoomCountX = roomCountX,
        .RoomCountY = roomCountY,
        .RoomIds = malloc(sizeof(s32) * roomCountX * roomCountY),
        .StartingRoomId = -1
    };
    
    assert(result.RoomIds);
    memset(result.RoomIds, -1, sizeof(s32) * roomCountX * roomCountY);

    u32 roomQueue[32];
    u32 roomQCount = 0;

    // Queue room in center of map
    Room* room = CreateNewRoom();
    roomQueue[roomQCount++] = room->Id;
    room->MapX = roomCountX / 2 - 1;
    room->MapY = roomCountY / 2 - 1;
    result.RoomIds[room->MapY * roomCountX + room->MapX] = room->Id;
    result.StartingRoomId = room->Id;
    room->IsVisited = true;

    u32 roomsPlaced = 0;
    while(roomsPlaced < roomsToPlace)
    {
        for (s32 queueIndex = 0; queueIndex < roomQCount && roomsPlaced < roomsToPlace; queueIndex++)
        {
            u32 roomId = roomQueue[queueIndex];
            Room* currentRoom = RoomById(roomId);
            
            u32 rand = RandU32Between(0, 3);
            Vec2 direction = V2(0, 0);
            switch (rand)
            {
            case 0:
                direction.x = -1;
                break;
            case 1:
                direction.y = -1;
                break;
            case 2:
                direction.x = 1;
                break;
            case 3:
                direction.y = 1;
                break;
            default:
                break;
            }

            s32 roomX = currentRoom->MapX + direction.x;
            s32 roomY = currentRoom->MapY + direction.y;

            // Skip outside of grid
            if (roomX < 0 || roomX >= roomCountX || roomY < 0 || roomY >= roomCountY)
            {
                continue;
            }

            u32 index = roomY * roomCountX + roomX;
            s32 roomCandidateId = result.RoomIds[index];
            
            // Skip rooms which are already set
            if (roomCandidateId == currentRoom->Id || roomCandidateId > -1) 
            {
                continue;
            }

            if (roomsPlaced < roomsToPlace)
            {
                Room* newRoom = CreateNewRoom();
                newRoom->MapX = roomX;
                newRoom->MapY = roomY;
                result.RoomIds[index] = newRoom->Id;
                roomQueue[roomQCount++] = newRoom->Id;
                ConnectRooms(currentRoom, newRoom);
                roomsPlaced++;
            }
        }
    }

    return result;
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
    Room* room = RoomById(gameState.CurrentRoomId);

    Vec2 result = RandVec2In(
        (Vec2){ 16, 16 },
        (Vec2){ room->TileCountX * TILE_PIXEL_SIZE - 16, room->TileCountY * TILE_PIXEL_SIZE - 16 }
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

// :bullets
Bullet* CreateBullet(Vec2 Position, Vec2 Velocity)
{
    for (s32 i = 0; i < BULLET_CAPACITY; i++)
    {
        Bullet* bullet = gameState.Bullets + i;
        if (bullet->TimeToLive <= 0)
        {
            bullet->TimeToLive = 3;
            bullet->Position = Position;
            bullet->Size = V2(8, 8);
            bullet->Velocity = Velocity;
            bullet->RoomId = gameState.CurrentRoomId;
            return bullet;
        }
    }

    assert(false);
}

// :bullets
void UpdateBullets(f32 delta)
{
    Room* room = RoomById(gameState.CurrentRoomId);

    for (s32 i = 0; i < BULLET_CAPACITY; i++)
    {
        Bullet* bullet = gameState.Bullets + i;
        if (bullet->TimeToLive <= 0 || bullet->RoomId != gameState.CurrentRoomId)
        {
            continue;
        }

        bullet->TimeToLive -= delta;

        Vec2 newPosition = Vec2Add(bullet->Position, Vec2Mulf(bullet->Velocity, delta));
        if (newPosition.x <= 0 || newPosition.x >= room->Size.x || newPosition.y <= 0 || newPosition.y >= room->Size.y)
        {
            bullet->TimeToLive = 0;
        }

        bullet->Position = newPosition;

        EntityCollisionInfo collision = EntityQueryCollision(newPosition, bullet->Size, ENTITY_ID_EMPTY);
        if (collision.CollisionInfo.Colliding)
        {
            if (collision.CollidingEntityId.Index == gameState.PlayerId.Index)
            {
                Entity* player = EntityById(gameState.PlayerId);
                bullet->TimeToLive = 0;
                EntityReceiveDamage(player, Vec2Normalize(bullet->Velocity), 2, 1);
            }
        }
    }
}

// :bullets
void RenderBullets()
{
    Texture* texture = GetTexture("bullet_8x8.png");

    for (s32 i = 0; i < BULLET_CAPACITY; i++)
    {
        Bullet* bullet = gameState.Bullets + i;
        if (bullet->TimeToLive <= 0 || bullet->RoomId != gameState.CurrentRoomId)
        {
            continue;
        }

        QuadDrawCmd* cmd = DrawTexture(bullet->Position, texture);
        cmd->ColorOverwrite = 1;
        cmd->Color = COLOR_WHITE;
        cmd->ZLayer = ZLayer_Entity0;
    }
}

// :items
void SetupItems()
{
    gameState.ItemData[ItemId_Sword] = (ItemData){
        .Id = ItemId_Sword,
        .Kind = ItemKind_Weapon,
        .Icon = GetTexture("icon_sword.png"),
        .Texture = GetTexture("sword.png")
    };
    
    gameState.ItemData[ItemId_Spear] = (ItemData){
        .Id = ItemId_Spear,
        .Kind = ItemKind_Weapon,
        .Icon = GetTexture("icon_spear.png"),
        .Texture = GetTexture("spear.png")
    };

    // :item :new
}

ItemData* ItemDataById(ItemId id)
{
    return &gameState.ItemData[id];
}
