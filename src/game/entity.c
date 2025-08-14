#include "entity.h"

typedef struct EntityState
{
    Entity Entities[ENTITY_CAPACITY];
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

void EntityDestroy(EntityId id)
{
    assert(id.Index < ENTITY_CAPACITY);

    Entity* entity = &entityState.Entities[id.Index];

    assert(entity->Flags & EntityFlag_Active);

    memset(entity, 0, sizeof(Entity));
    id.Generation++;
    entity->Id = id;
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
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Solid) || entity->Id.Index == toSkip.Index)
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