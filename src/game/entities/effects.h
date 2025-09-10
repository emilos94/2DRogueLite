#include "entity.h"

#ifndef EFFECTS_H
#define EFFECTS_H

Entity* EffectCreateSlash(Vec2 position, Vec2 direction);
Entity* EffectCreatePoof(Vec2 position);
Entity* EffectCreateGroundImpact(Vec2 position);

#endif