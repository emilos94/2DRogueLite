#include "AL/al.h"
#include "AL/alc.h"
#include "engine_internal.h"
#include "file_util.h"

#ifndef SOUND_H
#define SOUND_H

#define SOUND_SOURCE_NAME_CAPACITY 128
typedef struct SoundSource {
	char Name[SOUND_SOURCE_NAME_CAPACITY];
	ALuint Buffer;
	ALuint Source;
    boolean Valid;
} SoundSource;

void SoundInit(void);
SoundSource SoundLoad(const char* path);
void SoundPlay(SoundSource* sound);
void SoundSetLoop(SoundSource* sound, boolean loop);

#endif