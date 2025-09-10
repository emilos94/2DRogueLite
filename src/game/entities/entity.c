#include "entity.h"

typedef struct EntityState
{
    Entity Entities[ENTITY_CAPACITY];

    u32 EntityEventListenerCount;
    OnEntityEvent EntityEventListeners[ENTITY_EVENT_LISTENER_CAPACITY];

    u32 EntityFrameEventCount;
    EntityEventData EntityFrameEvents[ENTITY_EVENTS_FRAME_CAPACITY];
} EntityState;
EntityState entityState = {0};

Entity* EntityCreate(EntityFlags flags)
{
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* e = &entityState.Entities[i];
        if (!e->Flags & EntityFlag_Active)
        {
            e->Id.Index = i;
            e->Flags = flags | EntityFlag_Active;
            e->RenderScale = (Vec2){1.0, 1.0};
            e->ShadowRenderScale = (Vec2){1.0, 1.0};
            EmitEntityEvent(e->Id, EntityEvent_Created);
            return e;
        }
    }

    assert(false);
}

Entity* EntityById(EntityId id)
{
    assert(id.Index < ENTITY_CAPACITY);

    Entity* entity = &entityState.Entities[id.Index];
    if (entity->Id.Generation == id.Generation)
    {
        return entity;
    }

    return NULL;
}

void EntityQueueDestroy(EntityId id)
{
    assert(id.Index < ENTITY_CAPACITY);

    Entity* entity = &entityState.Entities[id.Index];
    assert(entity->Flags & EntityFlag_Active);
    entity->QueuedForDestruction = true;
    EmitEntityEvent(entity->Id, EntityEvent_Destroyed);
}

Entity* GetEntities(void)
{
    return entityState.Entities;
}

EntityCollisionInfo EntityQueryCollision(Vec2 position, Vec2 size, EntityId toSkip)
{
    Vec2 maxPos = Vec2Add(position, size);
    EntityCollisionInfo result = {0};

    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entityState.Entities + i;
        if (!(entity->Flags & EntityFlag_Active) || 
              entity->Id.Index == toSkip.Index   ||
            !((entity->Flags & EntityFlag_Solid) || (entity->Flags & EntityFlag_Collider)))
        {
            continue;
        }

        Vec2 entityMinPos = Vec2Add(entity->Position, entity->BoundingBox.Offset);
        AABBCollisionInfo info = AABBCollision(
            position, 
            maxPos, 
            entityMinPos, 
            Vec2Add(entityMinPos, entity->BoundingBox.Size)
        );
        result.CollisionInfo = info;

        if (info.Colliding)
        {
            result.CollidingEntityId = entity->Id;
            return result;
        }
    }

    return result;
}

void EntityAnimationStart(Entity* entity, AnimationId id)
{
    assert(entity);
    assert(entity->Flags & EntityFlag_Animation);

    entity->AnimationId = id;
    entity->Animation.Data = GetAnimation(id);
    entity->Animation.Playing = true;
    entity->Animation.ShouldLoop = true;
    entity->Animation.Timer = 0;
}

u32 EntityQueryInRange(Vec2 position, f32 range, EntityId toSkip, Entity** dest, u32 destCapacity)
{
    u32 count = 0;
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entityState.Entities + i;   
        if (!(entity->Flags & EntityFlag_Active))
        {
            continue;
        }

        if (entity->Id.Index != toSkip.Index && Vec2Distance(position, entity->Position) <= range)
        {
            dest[count++] = entity;
            if (count == destCapacity - 1)
            {
                break;
            }
        }
    }

    return count;
}

void EntityFlash(Entity* entity, Vec3 color, f32 duration)
{
    assert(entity);

    entity->FlashColor = color;
    entity->FlashTime = duration;
    entity->FlashTimer = duration;
}

Vec2 EntityCenterPos(Entity* entity)
{
    Vec2 result = Vec2Add(entity->Position, Vec2Mulf(entity->Size, 0.5));
    return result;
}

void EntityReceiveDamage(Entity* entity, Vec2 direction, f32 knockBackAmount, f32 damage)
{
    assert(entity);

    entity->RenderScale.x = 1.7;
    entity->RenderScale.y = 0.4;
    EntityFlash(entity, (Vec3){1.0, 1.0, 1.0}, 1);
    entity->KnockBackAmount = knockBackAmount;
    entity->KnockBackDirection = direction;
    entity->Health -= damage;
}

void RegisterEntityEventListener(OnEntityEvent callback)
{
    assert(entityState.EntityEventListenerCount < ENTITY_EVENT_LISTENER_CAPACITY - 1);

    entityState.EntityEventListeners[entityState.EntityEventListenerCount++] = callback;
}

void EmitEntityEvent(EntityId id, EntityEvent event)
{
    if (entityState.EntityFrameEventCount == ENTITY_EVENTS_FRAME_CAPACITY - 1)
    {
        ResolveEntityFrameEvents();
    }

    entityState.EntityFrameEvents[entityState.EntityFrameEventCount++] = (EntityEventData){ .EntityId = id, .Event = event };
}

void ResolveEntityFrameEvents(void)
{
    for (u32 i = 0; i < entityState.EntityEventListenerCount; i++)
    {
        OnEntityEvent handler = entityState.EntityEventListeners[i];
        for(u32 j = 0; j < entityState.EntityFrameEventCount; j++)
        {
            EntityEventData eventData = entityState.EntityFrameEvents[j];
            (*handler)(eventData.EntityId, eventData.Event);
        }
    }

    entityState.EntityFrameEventCount = 0;
}

Direction DirectionOpposite(Direction direction)
{
    switch (direction)
    {
    case Direction_East: return Direction_West;
    case Direction_West: return Direction_East;
    case Direction_North: return Direction_South;
    case Direction_South: return Direction_North;
    default: assert(false);
    }
}
