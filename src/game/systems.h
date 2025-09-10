#include "entities/entity.h"
#include "game_internal.h"

#ifndef SYSTEMS_H
#define SYSTEMS_H

void MovementSystem(f32 delta);
void CollisionSystem(f32 delta);
void AnimationSystem(f32 delta);
void TimeToLiveSystem(f32 delta);
void EntityDestroySystem(f32 delta);

#endif