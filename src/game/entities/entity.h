#include "engine/engine.h"
#include "../assets.h"

#ifndef ENTITY_H
#define ENTITY_H

#define ENTITY_CAPACITY 1024
typedef struct EntityId
{
    u32 Index;
    u32 Generation;
} EntityId;
#define ENTITY_ID_EMPTY ((EntityId){0, 0})

enum EntityFlags
{
    EntityFlag_Active = 1 << 0,
    EntityFlag_Moving = 1 << 1,
    EntityFlag_Solid = 1 << 2,
    EntityFlag_Render = 1 << 3,
    EntityFlag_Animation = 1 << 4,
    EntityFlag_FlipXOnMove = 1 << 5,
    EntityFlag_IsEffect = 1 << 6,
    EntityFlag_HasTimeToLive = 1 << 7,
    EntityFlag_HasHealth = 1 << 8,
    EntityFlag_RenderShadow = 1 << 9,
    EntityFlag_Collider = 1 << 10,
};
typedef u64 EntityFlags;

enum EntityKind
{
    EntityKind_None,
    EntityKind_Player,
    EntityKind_Enemy,
    EntityKind_COUNT
};
typedef u32 EntityKind;

enum EntityEvent
{
    EntityEvent_Created,
    EntityEvent_Destroyed
};
typedef u32 EntityEvent;

#define ENTITY_EVENTS_FRAME_CAPACITY 64
#define ENTITY_EVENT_LISTENER_CAPACITY 32
typedef void (*OnEntityEvent)(EntityId,EntityEvent);
// note: these callbacks are mostly for systems etc to hook into to get notified of entity updates,
// not for registering and unregistering lots of logic
void RegisterEntityEventListener(OnEntityEvent callback);
void EmitEntityEvent(EntityId id, EntityEvent event);
void ResolveEntityFrameEvents(void);

typedef struct
{
    EntityId EntityId;
    EntityEvent Event;
} EntityEventData;

typedef struct BoundingBox
{
    Vec2 Offset;
    Vec2 Size;
} BoundingBox;

// Callback definitions
typedef struct Entity Entity;
typedef void (*EntityCallback)(Entity*, f32);
typedef void (*EntityCollisionCallback)(Entity*, Entity*);

enum Direction
{
    Direction_West = 1 << 0,
    Direction_North = 1 << 1,
    Direction_East = 1 << 2,
    Direction_South = 1 << 3
};
typedef u8 Direction;
Direction DirectionOpposite(Direction direction);

typedef struct RoomSwitchData
{
    s32 ConnectedRoom;
    Vec2 PlayerSpawnPosition;
    Direction Placement;
    EntityId ConnectedDoorwayId;
} RoomSwitchData;

typedef struct Entity
{
    EntityId Id;
    EntityFlags Flags;
    EntityKind Kind;

    Vec2 Position;
    Vec2 Size;
    Vec2 RenderScale;
    f32 RenderOffsetY;
    f32 Rotation;
    Texture* Texture;

    f32 InvulnerableTimer;

    boolean QueuedForDestruction;

    Animation Animation;
    AnimationId AnimationId;

    Vec2 Velocity;

    boolean FlipTextureX;

    BoundingBox BoundingBox;

    f32 Health;

    Direction DoorDirection;

    // Jumping
    boolean IsJumping;
    f32 JumpTimer;
    f32 JumpTime;
    f32 JumpCooldown;

    Vec2 JumpStartPos;
    Vec2 JumpTarget;

    f32 JumpUpVelocity;
    f32 JumpGravity;
    
    f32 JumpDriver;
    f32 JumpHeight;
    Vec2 ShadowRenderScale;
    
    // Flash
    f32 FlashTime;
    f32 FlashTimer;
    f32 FlashStrength;
    Vec3 FlashColor;

    // Knockback
    f32 KnockBackAmount;
    Vec2 KnockBackDirection;

    f32 AliveTimer;

    // For effects mostly
    boolean DeleteOnAnimationFinish;

    // Move into gamestate?
    // Note: Have weapon as standalone entity ?
    Vec2 WeaponAnchor;
    f32 WeaponRotation;
    f32 WeaponExtraRotation;
    boolean WeaponUp;
    boolean WeaponSwinging;
    f32 WeaponOffsetDriver;
    f32 WeaponSwingTimer;
    f32 WeaponSwipe;
    f32 SwingSpeed;
    f32 AttackTimer;
    f32 AttackCooldown;

    u32 RoomId;

    RoomSwitchData RoomSwitchData;

    // Callbacks
    EntityCallback OnEntityDestroy;
    EntityCallback CustomCallback;
    EntityCollisionCallback OnCollision;
} Entity;

Entity* GetEntities(void);
Entity* EntityCreate(EntityFlags flags);
Entity* EntityById(EntityId id);
void EntityQueueDestroy(EntityId id);

typedef struct EntityCollisionInfo
{
    AABBCollisionInfo CollisionInfo;
    EntityId CollidingEntityId;
} EntityCollisionInfo;

EntityCollisionInfo EntityQueryCollision(Vec2 position, Vec2 size, EntityId toSkip);

// Fills dest with at most destCapacity entity ptrs if they are in range of pos and != toSkip
// Returns number of entities found
u32 EntityQueryInRange(Vec2 position, f32 range, EntityId toSkip, Entity** dest, u32 destCapacity);

void EntityAnimationStart(Entity* entity, AnimationId id);
void EntityFlash(Entity* entity, Vec3 color, f32 duration);
Vec2 EntityCenterPos(Entity* entity);

#define ENTITIES_LOOP(name)\
for(u32 i = 0; i < ENTITY_CAPACITY; i++)\
for(Entity* name = GetEntities() + i; name; name = 0)\

//typedef void(*OnEntityDestroyCallback)(Entity* entity, GameState* gameState);

#endif