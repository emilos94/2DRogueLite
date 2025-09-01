#include "effects.h"

Entity* EffectCreateSlash(Vec2 position, Vec2 direction)
{
    Entity* effect = EntityCreate(EntityFlag_Animation | EntityFlag_IsEffect | EntityFlag_HasTimeToLive | EntityFlag_Moving);
    EntityAnimationStart(effect, AnimationId_EffectSlash);
    
    Vec2 offsetDirection = direction.x > 0 ? (Vec2) {direction.y, -direction.x} : (Vec2) {-direction.y, direction.x};
    Vec2 effectOffset = Vec2Mulf(offsetDirection, AnimationFrameWidth(&effect->Animation) / 2.0);
    effect->Position = Vec2Add(
        effectOffset,
        position
    );

    effect->AliveTimer = AnimationDuration(&effect->Animation);
    f32 movementSpeed = 100;
    effect->Velocity = Vec2Mulf(direction, movementSpeed);

    effect->Rotation = RadiansToDegrees(atan2f(direction.y, direction.x));
    effect->DeleteOnAnimationFinish = true;
    return effect;
}

Entity* EffectCreatePoof(Vec2 position)
{
    Entity* effect = EntityCreate(EntityFlag_Animation | EntityFlag_IsEffect);
    EntityAnimationStart(effect, AnimationId_EffectPoof);
    effect->DeleteOnAnimationFinish = true;
    Vec2 frameSize = (Vec2){ AnimationFrameWidth(&effect->Animation), AnimationFrameWidth(&effect->Animation) };
    effect->Position = Vec2Sub(position, Vec2Mulf(frameSize, 0.5));
    return effect;
}

Entity* EffectCreateGroundImpact(Vec2 position)
{
    Entity* effect = EntityCreate(EntityFlag_Animation | EntityFlag_IsEffect);
    EntityAnimationStart(effect, AnimationId_EffectGroundImpact);
    effect->DeleteOnAnimationFinish = true;
    Vec2 frameSize = (Vec2){ AnimationFrameWidth(&effect->Animation), AnimationFrameWidth(&effect->Animation) };
    effect->Position = Vec2Sub(position, Vec2Mulf(frameSize, 0.5));
    return effect;
}