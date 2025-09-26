#include "engine/engine.h"
#include "dirent.h"

#ifndef ASSETS_H
#define ASSETS_H

void AssetsLoad(void);
void AssetsDestroy(void);
void LoadSounds(void);

Texture* GetTexture(const char* path);
SoundSource* GetSound(const char* name);

enum AnimationId
{
    AnimationId_None,
    AnimationId_PlayerIdle,
    AnimationId_PlayerRun,
    AnimationId_EffectSlash,
    AnimationId_EffectPoof,
    AnimationId_EffectGroundImpact,
    AnimationId_EffectSpear,
    AnimationId_EffectSpearSmall,
    AnimationId_COUNT
};
typedef u32 AnimationId;

AnimationData* GetAnimation(AnimationId id);

#endif
