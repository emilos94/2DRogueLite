#include "game_internal.h"
#include "systems.h"
#include "assets.h"
#include "entities/player.h"
#include "rooms.h"

void GameInit()
{
    WindowCreate("Test", 1280, 720);
    RenderInit(100);
    
    Mat4 projection = Mat4Orthographic(0, RESOLUTION_WIDTH, 0, RESOLUTION_HEIGHT);
    RenderSetProjection(projection);
    AssetsLoad();
    
    pthread_t backgroundThread;
    s32 result = pthread_create(&backgroundThread, NULL, LoadResourcesBackground, (void*)(&gameState));
    
    gameState.IsRunning = true;

    // Register event listeners
    // RegisterEntityEventListener(RoomsOnEntityEvent);

    // Store index to start room
    Map map = GenerateMap(10, 10, 20);

    // Second pass, set tiles to block sides which are not connected
    for(s32 y = 0; y < map.RoomCountY; y++)
    for(s32 x = 0; x < map.RoomCountX; x++)
    {
        s32 index = y * map.RoomCountX + x;

        s32 roomId = map.RoomIds[index];
        if (roomId == -1)
        {
            continue;
        }
        
        Room* room = RoomById(roomId);
        RoomLoadTiles(room, "res/rooms/room_01.png");

        if (room->ConnectedRoomIds[RoomIdFromDirection(Direction_West)] < 0)
        {
            // Block left side
            for (s32 roomY = 0; roomY < room->TileCountY; roomY++)
            {
                room->Tiles[roomY * room->TileCountY] = TileType_Wall;
            }
        }
        
        if (room->ConnectedRoomIds[RoomIdFromDirection(Direction_East)] < 0)
        {
            // Block right side
            for (s32 roomY = 0; roomY < room->TileCountY; roomY++)
            {
                room->Tiles[roomY * room->TileCountY + (room->TileCountX - 1)] = TileType_Wall;
            }
        }
        
        if (room->ConnectedRoomIds[RoomIdFromDirection(Direction_North)] < 0)
        {
            // Block bottom side
            for (s32 roomX = 0; roomX < room->TileCountX; roomX++)
            {
                room->Tiles[(room->TileCountY - 1) * room->TileCountY + roomX] = TileType_Wall;
            }
        }
        
        if (room->ConnectedRoomIds[RoomIdFromDirection(Direction_South)] < 0)
        {
            // Block top side
            for (s32 roomX = 0; roomX < room->TileCountX; roomX++)
            {
                room->Tiles[roomX] = TileType_Wall;
            }
        }
    }

    gameState.CurrentRoomId = map.StartingRoomId;
    gameState.map = map;

    InitNewGame();
}

void GameUpdate(float delta)
{
    if (gameState.GameMode == GameMode_GameOver)
    {
        if (KeyPressed(GLFW_KEY_ENTER))
        {
            InitNewGame();
        }
    }
    else 
    {
        gameState.ElapsedTime += delta;
        
#ifdef DEBUG
        if (KeyDown(GLFW_KEY_LEFT_SHIFT) && KeyPressed(GLFW_KEY_C))
        {
            debugState.RenderCollisionShapes = !debugState.RenderCollisionShapes;
        }

        if (KeyDown(GLFW_KEY_LEFT_SHIFT) && KeyPressed(GLFW_KEY_M))
        {
            debugState.RenderMiniMap = !debugState.RenderMiniMap;
        }
#endif
        
        if (KeyPressed(GLFW_KEY_ESCAPE))
        {
            gameState.IsRunning = false;
        }
        
        Entity* player = EntityById(gameState.PlayerId);
        if (player)
        {
            PlayerInput(&gameState, player, delta);
            PlayerUpdate(&gameState, player, delta);
        }

        CollisionSystem(delta);
        MovementSystem(delta);
        AnimationSystem(delta);
        TimeToLiveSystem(delta);

        EntitiesUpdate(delta);

        ResolveEntityFrameEvents();
        EntityDestroySystem(delta);
    }
}

void GameRender(float delta)
{   
    Entity* player = EntityById(gameState.PlayerId);

    RenderStartFrame();

    if (player)
    {
        Room* currentRoom = RoomById(gameState.CurrentRoomId);

        gameState.CameraTargetPosition = Vec2Sub(player->Position, (Vec2){RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2});
        Vec2 cameraMinPos = (Vec2) { .x = 0, .y = 0 };
        Vec2 cameraMaxPos =  (Vec2) { .x = currentRoom->Size.x - RESOLUTION_WIDTH, .y =currentRoom->Size.y - RESOLUTION_HEIGHT };
        gameState.CameraTargetPosition = Vec2Clamp(
            gameState.CameraTargetPosition, cameraMinPos, cameraMaxPos
        );
        gameState.CameraPosition = Vec2Lerp(gameState.CameraPosition, gameState.CameraTargetPosition, 10.0 * delta);
        RenderSetCameraPos(gameState.CameraPosition);
    }

    Texture* grassTexture = GetTexture("grass_240x240.png");
    QuadDrawCmd* grassTexture0 = DrawTexture(V2(0, 0), grassTexture);
    grassTexture0->ZLayer = ZLayer_Ground;
    QuadDrawCmd* grassTexture1 = DrawTexture(V2(0, 240), grassTexture);
    grassTexture1->ZLayer = ZLayer_Ground;
    QuadDrawCmd* grassTexture2 = DrawTexture(V2(240, 0), grassTexture);
    grassTexture2->ZLayer = ZLayer_Ground;
    QuadDrawCmd* grassTexture3 = DrawTexture(V2(240, 240), grassTexture);
    grassTexture3->ZLayer = ZLayer_Ground;
    
    Room* currentRoom = RoomById(gameState.CurrentRoomId);
    DrawRoomTiles(currentRoom);

    Entity* entities = GetEntities();       
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
        
        QuadDrawCmd* cmd = 0;
        if (entity->Flags & EntityFlag_Animation)
        {
            cmd = DrawAnimation(entity->Position, &entity->Animation);
        }
        else if (entity->Flags & EntityFlag_Render)
        {
            cmd = DrawTexture(entity->Position, entity->Texture);
        }
        
        if (cmd)
        {
            cmd->FlipTextureX = entity->FlipTextureX;
            cmd->ZLayer = 2;
            cmd->Rotation = entity->Rotation;
            cmd->Size = Vec2Mul(cmd->Size, entity->RenderScale);

            if (entity->FlashTimer > 0)
            {
                cmd->Color = entity->FlashColor;
                cmd->ColorOverwrite = entity->FlashStrength;
            }

            cmd->Position.y += entity->RenderOffsetY;

            if (entity->Flags & EntityFlag_IsEffect)
            {
                cmd->ZLayer = 1;
            }

            if (entity->Flags & EntityFlag_RenderShadow)
            {
                QuadDrawCmd* shadow = DrawTexture(entity->Position, GetTexture("shadow_small.png"));
                shadow->Position.y -= 3;
                shadow->ZLayer = 1;
                shadow->Alpha = 0.5;
            }
        }

        if (entity->Id.Index == gameState.PlayerId.Index)
        {
            QuadDrawCmd* drawCmd = DrawTexture(entity->WeaponAnchor, GetTexture("sword.png"));
            drawCmd->Rotation = entity->WeaponRotation;
            drawCmd->ZLayer = 2;
        }

#ifdef DEBUG
        if (debugState.RenderCollisionShapes && entity->Flags & EntityFlag_Solid)
        {
            QuadDrawCmd* collisionShape = DrawQuad(
                Vec2Add(entity->Position, entity->BoundingBox.Offset), 
                entity->BoundingBox.Size, 
                (Vec3){0.8, 0.2, 0.2}
            );
            collisionShape->ZLayer = 3;
        }
#endif
            
    }
    RenderEndFrame();

    // Draw ui in separate draw call with 0 camera pos
    RenderStartFrame();
    RenderSetCameraPos((Vec2){0, 0});
    
#ifdef DEBUG
    if (debugState.RenderMiniMap)
    {
        u32 roomSize = 8;
        u32 roomPadding = 2;
        u32 mapSize = 104;

        Vec2 mapBottomLeft = V2(RESOLUTION_WIDTH / 2 - mapSize / 2, RESOLUTION_HEIGHT / 2 - mapSize /2);
        QuadDrawCmd* backGround = DrawQuad(mapBottomLeft, V2(mapSize, mapSize), (Vec3){0, 0, 0});
        backGround->ZLayer = ZLayer_UI0;

        for(s32 y = 0; y < gameState.map.RoomCountY; y++)
        for(s32 x = 0; x < gameState.map.RoomCountX; x++)
        {
            s32 roomId = gameState.map.RoomIds[y * gameState.map.RoomCountX + x];
            
            Vec3 color = (Vec3){0.9, 0.4, 0.2};
            if (roomId == -1)
            {
                color = (Vec3){0.3, 0.3, 0.3};
            }
            else if (roomId == gameState.CurrentRoomId)
            {
                color = (Vec3){0.4, 0.4, 0.9};
            }

            QuadDrawCmd* cmd = DrawQuad(
                Vec2Add(V2(x * roomPadding + x * roomSize, y * roomPadding + y * roomSize), mapBottomLeft),
                V2(roomSize, roomSize),
                color
            );
            cmd->ZLayer = ZLayer_UI1;
        }
    }
#endif

    // ui
    // player health bar
    if (player)
    {
        u32 width = 64;
        u32 height = 10;
        u32 padding = 2;
        QuadDrawCmd* healthBarBack = DrawQuad(
            (Vec2) { padding, RESOLUTION_HEIGHT - (height + padding) },
            (Vec2) { width, height},
            (Vec3) {1.0, 0.2, 0.2}
        );
        healthBarBack->ZLayer = ZLayer_UI0;
        
        QuadDrawCmd* healthBarFront= DrawQuad(
            (Vec2) { padding, RESOLUTION_HEIGHT - (height + padding) },
            (Vec2) { width * (player->Health) / 5, height},
            (Vec3) { 0.2, 1.0, 0.2}
        );
        healthBarFront->ZLayer = ZLayer_UI1;
    }
    RenderEndFrame();
}

void GameDestroy()
{
    AssetsDestroy();
    free(gameState.map.RoomIds);
}

boolean GameRunning()
{
    return gameState.IsRunning;
}
