#include "game_internal.h"
#include "systems.h"
#include "assets.h"
#include "player.h"

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

    // :player
    Entity* player = EntityCreate(EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Animation);
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
    gameState.PlayerId = player->Id;

    Entity* slime = EntityCreate(EntityFlag_Moving | EntityFlag_Solid | EntityFlag_Render);
    slime->Texture = GetTexture("slime.png");
    slime->Position = (Vec2){200, 50};
    slime->BoundingBox = (BoundingBox){ .Offset = {5, 0}, .Size = {6, 6} };

    Entity* leftWall = EntityCreate(EntityFlag_Solid);
    leftWall->Position = (Vec2){0, 0};
    leftWall->BoundingBox = (BoundingBox) { .Offset = {0, 0}, .Size = {8, RESOLUTION_HEIGHT } };

    Entity* rightWall = EntityCreate(EntityFlag_Solid);
    rightWall->Position = (Vec2){RESOLUTION_WIDTH - 8, 0};
    rightWall->BoundingBox = (BoundingBox) { .Offset = {0, 0}, .Size = {8, RESOLUTION_HEIGHT } };

    Entity* topWall = EntityCreate(EntityFlag_Solid);
    topWall->Position = (Vec2){0, RESOLUTION_HEIGHT - 16};
    topWall->BoundingBox = (BoundingBox) { .Offset = {0, 0}, .Size = {RESOLUTION_WIDTH, 16 } };
    
    Entity* bottomWall = EntityCreate(EntityFlag_Solid);
    bottomWall->Position = (Vec2){0, 0};
    bottomWall->BoundingBox = (BoundingBox) { .Offset = {0, 0}, .Size = {RESOLUTION_WIDTH, 8 } };
}

void GameUpdate(float delta)
{
    gameState.ElapsedTime += delta;

    Entity* player = EntityById(gameState.PlayerId);

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

    PlayerInput(&gameState, player, delta);
    PlayerUpdate(&gameState, player, delta);

    MovementSystem(delta);
}

void GameRender(float delta)
{   
    QuadDrawCmd* grass = DrawTexture((Vec2){0, 0}, gameState.GrassTexture);
    grass->ZLayer = 0;
    QuadDrawCmd* walls = DrawTexture((Vec2){0, 0}, gameState.WallsTexture);
    walls->ZLayer = 1;

    Entity* entities = GetEntities();       
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {   
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active))
        {
            continue;
        }
        
        if (entity->Flags & EntityFlag_Animation)
        {
            QuadDrawCmd* cmd = DrawAnimation(entity->Position, &entity->Animation);
            cmd->FlipTextureX = entity->FlipTextureX;
            cmd->ZLayer = 2;
        }
        else if (entity->Flags & EntityFlag_Render)
        {
            QuadDrawCmd* cmd = DrawTexture(entity->Position, entity->Texture);
            cmd->FlipTextureX = entity->FlipTextureX;
            cmd->ZLayer = 2;
        }

        if (entity->Id.Index == gameState.PlayerId.Index)
        {
            QuadDrawCmd* cmd = DrawTexture(entity->WeaponAnchor, GetTexture("sword.png"));
            cmd->Rotation = entity->WeaponRotation;
            cmd->ZLayer = 2;
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
