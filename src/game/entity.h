#include "engine/engine.h"
#include "assets.h"

#ifndef ENTITY_H
#define ENTITY_H

#define ENTITY_CAPACITY 1024

typedef struct EntityId
{
    u32 Index;
    u32 Generation;
} EntityId;

enum EntityFlags
{
    EntityFlag_Active = 1 << 0,
    EntityFlag_Moving = 1 << 1,
    EntityFlag_Solid = 1 << 2,
    EntityFlag_Render = 1 << 3,
    EntityFlag_Animation = 1 << 4,
    EntityFlag_FlipXOnMove = 1 << 5,
};
typedef u64 EntityFlags;

typedef struct BoundingBox
{
    Vec2 Offset;
    Vec2 Size;
} BoundingBox;

typedef struct Entity
{
    EntityId Id;
    EntityFlags Flags;

    Vec2 Position;
    Vec2 Size;
    Texture* Texture;

    Animation Animation;
    AnimationId AnimationId;

    Vec2 Velocity;

    boolean FlipTextureX;

    BoundingBox BoundingBox;

    // Move into gamestate?
    Vec2 WeaponAnchor;
    f32 WeaponRotation;
    f32 WeaponExtraRotation;
    boolean WeaponUp;
    boolean WeaponSwinging;
    f32 WeaponOffsetDriver;
    f32 WeaponSwingTimer;
    f32 WeaponSwipe;
    f32 SwingSpeed;
} Entity;

Entity* GetEntities(void);
Entity* EntityCreate(EntityFlags flags);
Entity* EntityById(EntityId id);
void EntityDestroy(EntityId id);

typedef struct EntityCollisionInfo
{
    AABBCollisionInfo CollisionInfo;
    EntityId CollidingEntityId;
} EntityCollisionInfo;

EntityCollisionInfo EntityQueryCollision(Vec2 position, Vec2 size, EntityId toSkip);

#endif