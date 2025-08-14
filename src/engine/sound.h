#include "AL/al.h"
#include "AL/alc.h"
#include "engine_internal.h"
#include "file_util.h"

#ifndef SOUND_H
#define SOUND_H

typedef struct SoundSource {
	ALuint Buffer;
	ALuint Source;
    boolean Valid;
} SoundSource;

void SoundInit(void);
SoundSource SoundLoad(const char* path);
void SoundPlay(SoundSource* sound);
void SoundSetLoop(SoundSource* sound, boolean loop);

#endif