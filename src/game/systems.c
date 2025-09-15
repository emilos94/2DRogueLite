#include "systems.h"

void MovementSystem(f32 delta)
{
    Room* currentRoom = RoomById(gameState.CurrentRoomId);
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Moving))
        {
            continue;
        }

        if (entity->RoomId != gameState.CurrentRoomId)
        {
            continue;
        }

        Vec2 newPosition = Vec2Add(entity->Position, Vec2Mulf(entity->Velocity, delta));
        
        if (entity->Flags & EntityFlag_Solid)
        {
            EntityCollisionInfo collision = EntityQueryCollision(Vec2Add(newPosition, entity->BoundingBox.Offset), entity->BoundingBox.Size, entity->Id);

            if (collision.CollisionInfo.Colliding) 
            {
                Entity* collidingEntity = EntityById(collision.CollidingEntityId);
                if (collidingEntity && collidingEntity->Flags & EntityFlag_Solid)
                {
                    newPosition = Vec2Add(newPosition, collision.CollisionInfo.SeperationVector);
                }
            }
        }

        TileType tile = RoomGetTileAt(currentRoom, newPosition);
        if (tile != TileType_Wall)
        {
            entity->Position = newPosition;
        }
            
        if (entity->Flags & EntityFlag_FlipXOnMove)
        {
            if (entity->FlipTextureX && entity->Velocity.x > 0)
            {
                entity->FlipTextureX = false;
            }
            else if (!entity->FlipTextureX && entity->Velocity.x < 0)
            {
                entity->FlipTextureX = true;
            }
        }
    }
}

void CollisionSystem(f32 delta)
{
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Collider))
        {
            continue;
        }
        
        if (entity->RoomId != gameState.CurrentRoomId)
        {
            continue;
        }

        EntityCollisionInfo collision = EntityQueryCollision(Vec2Add(entity->Position, entity->BoundingBox.Offset), entity->BoundingBox.Size, entity->Id);
        if (collision.CollisionInfo.Colliding && entity->OnCollision)
        {

            (*entity->OnCollision)(entity, EntityById(collision.CollidingEntityId));
        }
    }
}

void AnimationSystem(f32 delta)
{
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Animation))
        {
            continue;
        }
        
        if (entity->RoomId != gameState.CurrentRoomId)
        {
            continue;
        }

        AnimationUpdate(&entity->Animation, delta);

        if (entity->DeleteOnAnimationFinish && entity->Animation.JustFinished)
        {
            EntityQueueDestroy(entity->Id);
        }
    }
}

void TimeToLiveSystem(f32 delta)
{
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_HasTimeToLive))
        {
            continue;
        }

        entity->AliveTimer -= delta;
        if (entity->AliveTimer <= 0)
        {
            entity->AliveTimer = 0;
            EntityQueueDestroy(entity->Id);
        }
    }
}

void EntityDestroySystem(f32 delta)
{
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !entity->QueuedForDestruction)
        {
            continue;
        }

        if (entity->OnEntityDestroy)
        {
            (*entity->OnEntityDestroy)(entity, delta);
        }

        EntityId id = entity->Id;
        memset(entity, 0, sizeof(Entity));
        id.Generation++;
        entity->Id = id;
    }
}

void EntityJumpingSystem(f32 delta)
{
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Jump) || !entity->IsJumping)
        {
            continue;
        }

        entity->JumpDriver += delta;
        f32 progress = entity->JumpDriver / entity->JumpTime;
        entity->Position = Vec2Lerp(entity->JumpStartPos, entity->JumpTarget, progress);

        // parabular
        entity->RenderOffsetY = -4.0 * entity->JumpHeight * (progress * progress - progress);

        if (entity->JumpDriver >= entity->JumpTime)
        {
            entity->RenderOffsetY = 0;
            entity->JumpDriver = 0;
            entity->IsJumping = false;
            entity->RenderScale.x = 1.7;
            entity->RenderScale.y = 0.4;

            if (entity->OnLandFromJumpCallback)
            {
                (*entity->OnLandFromJumpCallback)(entity, delta);
            }
        }
    }
}


void CollectibleSystem(f32 delta)
{
    const f32 delayBeforePickupTime = 1.0;
    Entity* player = EntityById(gameState.PlayerId);

    if (!player)
    {
        return;
    }

    ENTITIES_LOOP(entity)
    {
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Collectible))
        {
            continue;
        }

        if (gameState.ElapsedTime - entity->CreatedTime <= delayBeforePickupTime)
        {
            continue;
        }

        if (Vec2Distance(EntityCenterPos(player), entity->Position) <= entity->PickupRange || entity->IsBeingPickedUp)
        {
            if (!entity->IsBeingPickedUp)
            {
                entity->IsBeingPickedUp = true;
            }

            entity->Position = Vec2Lerp(entity->Position, EntityCenterPos(player), delta * 10);
        }
    }
}