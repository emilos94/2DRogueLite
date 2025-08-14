#include "systems.h"

void MovementSystem(f32 delta)
{
    Entity* entities = GetEntities();
    for (s32 i = 0; i < ENTITY_CAPACITY; i++)
    {
        Entity* entity = entities + i;
        if (!(entity->Flags & EntityFlag_Active) || !(entity->Flags & EntityFlag_Moving))
        {
            continue;
        }

        Vec2 newPosition = Vec2Add(entity->Position, Vec2Mulf(entity->Velocity, delta));
        
        if (entity->Flags & EntityFlag_Solid)
        {
            EntityCollisionInfo collision = EntityQueryCollision(Vec2Add(newPosition, entity->BoundingBox.Offset), entity->BoundingBox.Size, entity->Id);
            if (collision.CollisionInfo.Colliding)
            {
                newPosition = Vec2Add(newPosition, collision.CollisionInfo.SeperationVector);
            }
        }
            
        entity->Position = newPosition;

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
