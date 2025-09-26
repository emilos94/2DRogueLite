#include "player.h"

void SwordAttack(Entity* player)
{
    player->AttackTimer = 0;
    player->WeaponSwipe *= -1.0;
    player->WeaponAttacking = true;
    player->WeaponAttackTimer = player->AttackSpeed;

    SoundPlay(GetSound("strike_blade_medium_003.wav"));
    Vec2 attackDirection = Vec2Direction(player->Position, MousePosWorld());

    Vec2 effectPosition = player->Position;
    if (attackDirection.x < 0)
    {
        effectPosition.x -= 15;
    }
    if (attackDirection.y < 0)
    {
        effectPosition.y -= 5;
    }

    EffectCreateSlash(effectPosition, attackDirection);

    Vec2 playerCenterPos = Vec2Add(
        player->Position,
        Vec2Mulf(player->Size, 0.5)
    );

    f32 attackRange = 30;
    Entity* result[10];
    u32 count = EntityQueryInRange(playerCenterPos, attackRange, player->Id, &result[0], 10);
    for (s32 i = 0; i < count; i++)
    {
        // todo: Add angle/direction check as well
        Entity* entity = result[i];
        if (entity->Kind == EntityKind_Enemy && !entity->IsJumping)
        {
            Vec2 directionToEnemy = Vec2Direction(playerCenterPos, entity->Position);
            f32 angleRadians = cosf(30); 
            if (Vec2Dot(attackDirection, directionToEnemy) >= angleRadians)
            {
                EntityReceiveDamage(entity, attackDirection, 3.5, 1);
            }
        }
    }
}

void SwordUpdate(Entity* player, float delta)
{
    f32 swingProgress = (player->AttackSpeed - player->WeaponAttackTimer) / player->AttackSpeed;
    f32 swipe = player->WeaponSwipe;
    if (player->FlipTextureX)
    {
        swipe *= -1.0f;
    }
    player->WeaponOffsetDriver = Lerp(player->WeaponOffsetDriver, swipe, swingProgress);

    Vec2 direction = Vec2Direction(player->Position, MousePosWorld());
    f32 directionAngle = atan2f(direction.y, direction.x) + player->WeaponOffsetDriver * 2;
    player->WeaponAnchor = (Vec2) {
        .x = player->Position.x + cos(directionAngle) * 10,
        .y = player->Position.y + sin(directionAngle) * 10
    };

    if (swipe < 0)
    {
        player->WeaponExtraRotation = Lerp(player->WeaponExtraRotation, -180, swingProgress);
        player->WeaponRotation = RadiansToDegrees(directionAngle) + player->WeaponExtraRotation;
    }
    else 
    {
        player->WeaponExtraRotation = Lerp(player->WeaponExtraRotation, 0, swingProgress);
        player->WeaponRotation = RadiansToDegrees(directionAngle) + player->WeaponExtraRotation;
    }
    
    Vec2 weaponOffset = (Vec2) { .x = 3, .y = 0 };
    if (player->FlipTextureX)
    {
        weaponOffset = (Vec2) {.x = 7, .y = 0 };
    }
    player->WeaponAnchor = Vec2Add(player->WeaponAnchor, weaponOffset);
}

void SpearAttack(Entity* player)
{
    Vec2 mousePosWorld = MousePosWorld();
    Vec2 direction = Vec2Direction(player->Position, mousePosWorld);

    Vec2 impactPos = Vec2Add(player->Position, Vec2Mulf(direction, 32));

    player->WeaponAncorOffset = Vec2Sub(impactPos, player->Position);

    Entity* effect = EffectOneshotAnimation(impactPos, AnimationId_EffectSpearSmall);
    effect->Rotation = RadiansToDegrees(atan2f(direction.y, direction.x));
    effect->Velocity = Vec2Mulf(direction, 100);
    effect->Flags |= EntityFlag_Moving;

    Entity* results[10];
    u32 found = EntityQueryInRange(impactPos, 16, player->Id, &results[0], 10);
    u32 hitCount = 0;
    for (s32 i = 0; i < found; i++)
    {
        Entity* entity = results[i];
        if (entity->Kind == EntityKind_Enemy)
        {
            hitCount++;
            EntityReceiveDamage(entity, direction, 3, 1);
        }
    }

    if (hitCount > 0)
    {
        SoundPlay(GetSound("strike_blade_medium_003.wav"));
    }
}

void SpearUpdate(Entity* player, f32 delta)
{
    Vec2 mousePosWorld = MousePosWorld();
    Vec2 direction = Vec2Direction(player->Position, mousePosWorld);
    f32 directionAngle = atan2f(direction.y, direction.x);

    player->WeaponAncorOffset = Vec2Lerp(player->WeaponAncorOffset, V2(0, 0), 10 * delta);

    player->WeaponAnchor = mousePosWorld.x > player->Position.x ? Vec2Add(player->Position, V2(6, 0)) : player->Position;
    player->WeaponAnchor = Vec2Add(player->WeaponAnchor, player->WeaponAncorOffset);
    player->WeaponAnchor.y -= 5;

    player->WeaponRotation = RadiansToDegrees(directionAngle) - 90;
}

void PlayerInput(GameState* gameState, Entity* player, f32 delta)
{
    f32 playerSpeed = 85;
    Vec2 movementVector = {0};
    if (KeyDown(GLFW_KEY_A))
    {
        movementVector.x -= 1.0;
    }
    if (KeyDown(GLFW_KEY_D))
    {
        movementVector.x += 1.0;
    }
    if (KeyDown(GLFW_KEY_S))
    {
        movementVector.y -= 1.0;
    }
    if (KeyDown(GLFW_KEY_W))
    {
        movementVector.y += 1.0;
    }
    movementVector = Vec2Normalize(movementVector);
    player->Velocity = Vec2Mulf(movementVector, playerSpeed);

    player->FlipTextureX = MousePosWorld().x < player->Position.x;

    // Attack
    if (MouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && player->AttackTimer >= player->AttackCooldown && !gameState->UIHovered)
    {
        ActionBarSlot* slot = NULL;
        if (gameState->ActionBarSelectedIndex != -1)
        {
            slot = &gameState->ActionBarSlots[gameState->ActionBarSelectedIndex];
        }

        if (slot)
        {
            switch(slot->ItemId)
            {
                case ItemId_Sword: SwordAttack(player); break;
                case ItemId_Spear: SpearAttack(player); break;
                default: break;
            }
            
            player->AttackTimer = 0;
            player->WeaponAttacking = true;
            player->WeaponAttackTimer = player->AttackSpeed;
        }
    }

}

void PlayerUpdate(GameState* gameState, Entity* player, f32 delta)
{
    player->AttackTimer += delta;

    // animation
    f32 velocityMagnitude = Vec2Magnitude(player->Velocity);
    if (velocityMagnitude > 0 && player->AnimationId != AnimationId_PlayerRun)
    {
        EntityAnimationStart(player, AnimationId_PlayerRun);
    }
    else if (F32Equals(velocityMagnitude, 0) && player->AnimationId != AnimationId_PlayerIdle)
    {
        EntityAnimationStart(player, AnimationId_PlayerIdle);
    }    

    // Weapon swinging
    if (player->WeaponAttacking)
    {
        player->WeaponAttackTimer -= delta;
        if (player->WeaponAttackTimer <= 0)
        {
            player->WeaponAttackTimer = 0;
            player->WeaponAttacking = false;
        }
    }

    ActionBarSlot* slot = NULL;
    if (gameState->ActionBarSelectedIndex != -1)
    {
        slot = &gameState->ActionBarSlots[gameState->ActionBarSelectedIndex];
    }

    if (slot)
    {
        switch(slot->ItemId)
        {
            case ItemId_Sword: SwordUpdate(player, delta); break;
            case ItemId_Spear: SpearUpdate(player, delta); break;
            default: break;
        }
    }

    // :room :switching
    Room* currentRoom = RoomById(gameState->CurrentRoomId);
    if (player->Position.x <= 0)
    {
        s32 roomId = currentRoom->ConnectedRoomIds[RoomIdFromDirection(Direction_West)];
        if (roomId != -1)
        {
            Room* westRoom = RoomById(roomId);
            SwitchToRoom(Direction_East, westRoom);
        }
    }
    else if (player->Position.x + player->Size.x >= currentRoom->TileCountX * TILE_PIXEL_SIZE)
    {
        s32 roomId = currentRoom->ConnectedRoomIds[RoomIdFromDirection(Direction_East)];
        if (roomId != -1)
        {
            Room* eastRoom = RoomById(roomId);
            SwitchToRoom(Direction_West, eastRoom);
        }
    }
    else if (player->Position.y + player->Size.y >= currentRoom->TileCountY * TILE_PIXEL_SIZE)
    {
        s32 roomId = currentRoom->ConnectedRoomIds[RoomIdFromDirection(Direction_North)];
        if (roomId != -1)
        {
            Room* northRoom = RoomById(roomId);
            SwitchToRoom(Direction_South, northRoom);
        }
    }
    else if (player->Position.y <= 0)
    {
        s32 roomId = currentRoom->ConnectedRoomIds[RoomIdFromDirection(Direction_South)];
        if (roomId != -1)
        {
            Room* southRoom = RoomById(roomId);
            SwitchToRoom(Direction_North, southRoom);
        }
    }
}