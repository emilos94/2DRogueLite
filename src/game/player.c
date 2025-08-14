#include "player.h"

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

    if (MouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        player->WeaponSwipe *= -1.0;
        player->WeaponSwinging = true;
        player->WeaponSwingTimer = player->SwingSpeed;

        SoundPlay(&gameState->TestSound);
    }
}

void PlayerUpdate(GameState* gameState, Entity* player, f32 delta)
{
    // animation
    f32 velocityMagnitude = Vec2Magnitude(player->Velocity);
    if (velocityMagnitude > 0 && player->AnimationId != AnimationId_PlayerRun)
    {
        player->Animation.Data = GetAnimation(AnimationId_PlayerRun);
        player->Animation.Timer = 0;
        player->AnimationId = AnimationId_PlayerRun;
    }
    else if (F32Equals(velocityMagnitude, 0) && player->AnimationId != AnimationId_PlayerIdle)
    {
        player->Animation.Data = GetAnimation(AnimationId_PlayerIdle);
        player->Animation.Timer = 0;
        player->AnimationId = AnimationId_PlayerIdle;
    }    
    UpdateAnimation(&player->Animation, delta);

    // Weapon swinging
    if (player->WeaponSwinging)
    {
        player->WeaponSwingTimer -= delta;
        if (player->WeaponSwingTimer <= 0)
        {
            player->WeaponSwingTimer = 0;
            player->WeaponSwinging = false;
        }
    }
    
    f32 swingProgress = (player->SwingSpeed - player->WeaponSwingTimer) / player->SwingSpeed;
    f32 swipe = player->WeaponSwipe;
    if (player->FlipTextureX)
    {
        swipe *= -1.0f;
    }
    player->WeaponOffsetDriver = Lerp(player->WeaponOffsetDriver, swipe, swingProgress);

    Vec2 direction = Vec2Direction(player->Position, MousePosResolution());
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
    player->FlipTextureX = MousePosResolution().x < player->Position.x;
    
    Vec2 weaponOffset = (Vec2) { .x = 3, .y = 0 };
    if (player->FlipTextureX)
    {
        weaponOffset = (Vec2) {.x = 7, .y = 0 };
    }
    player->WeaponAnchor = Vec2Add(player->WeaponAnchor, weaponOffset);
}