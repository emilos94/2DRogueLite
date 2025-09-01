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
    gameState.GrassTexture = GetTexture("grass.png");
    gameState.WallsTexture = GetTexture("walls.png");

    // Register event listeners
    RegisterEntityEventListener(RoomsOnEntityEvent);
    
    // init room:
    Room* initialRoom = CreateNewRoom();
    gameState.CurrentRoomId = initialRoom->Id;
    RoomLoadTiles(initialRoom, "res/rooms/test_map01.png");

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
    
    // ui
    // player health bar
    if (player)
    {
        gameState.CameraTargetPosition = Vec2Sub(player->Position, (Vec2){RESOLUTION_WIDTH / 2, RESOLUTION_HEIGHT / 2});
        Vec2 cameraMinPos = (Vec2) { .x = 0, .y = 0 };
        Vec2 cameraMaxPos =  (Vec2) { .x = ROOM_MAX_TILE_WIDTH * TILE_PIXEL_SIZE - RESOLUTION_WIDTH, .y = ROOM_MAX_TILE_HEIGHT * TILE_PIXEL_SIZE - RESOLUTION_HEIGHT };
        gameState.CameraTargetPosition = Vec2Clamp(
            gameState.CameraTargetPosition, cameraMinPos, cameraMaxPos
        );
        gameState.CameraPosition = Vec2Lerp(gameState.CameraPosition, gameState.CameraTargetPosition, 10.0 * delta);
        RenderSetCameraPos(gameState.CameraPosition);

        u32 width = 64;
        u32 height = 10;
        u32 padding = 2;
        QuadDrawCmd* healthBarBack = DrawQuad(
            (Vec2) { padding, RESOLUTION_HEIGHT - (height + padding) },
            (Vec2) { width, height},
            (Vec3) {1.0, 0.2, 0.2}
        );
        healthBarBack->ZLayer = -2;
        
        QuadDrawCmd* healthBarFront= DrawQuad(
            (Vec2) { padding, RESOLUTION_HEIGHT - (height + padding) },
            (Vec2) { width * (player->Health) / 5, height},
            (Vec3) { 0.2, 1.0, 0.2}
        );
        healthBarFront->ZLayer = -1;
    }
    
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
}

void GameDestroy()
{
    AssetsDestroy();
}

boolean GameRunning()
{
    return gameState.IsRunning;
}
