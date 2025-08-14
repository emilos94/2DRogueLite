#include "sound.h"
#include "file_util.h"

#define OpenAL_ErrorCheck(message)\
do {\
	ALenum error = alGetError();\
	if (error != AL_NO_ERROR){\
        printf("OpenAL Error: %d, msg: %s\n", error, message);\
	}\
} while(0);

#define AL_CALL(function_call)\
function_call;\
OpenAL_ErrorCheck(#function_call)

#pragma pack(push, 1)
typedef struct
{
    u8 Riff[4];
    u32 Size;
    u8 Wave[4];
} WAVRiffHeader;

typedef struct 
{
    u8 Id[4];
    u32 Size;
} WAVChunkHeader;

typedef struct
{
    u16 AudioFormat;
    u16 NumChannels;
    u32 SampleRate;
    u32 ByteRate;
    u16 BlockAlign;
    u16 BitsPerSample;
} WAVFormatHeader;

#pragma pack(pop)

typedef struct 
{
    boolean IsInitialized;
} SoundState;
SoundState soundState = {0};

void SoundInit(void)
{
    const ALCchar* defaultDeviceString = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceString);

	if (!device) 
    {
		assert(false && "Failed to load default al device");
	}

    printf("OpenAL device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));

	ALCcontext* context = alcCreateContext(device, NULL);

	if (!alcMakeContextCurrent(context)) 
    {
		assert(false && "Failed to make OpenAL context the current context");
	}

	OpenAL_ErrorCheck("Setup OpenAl context");

	AL_CALL(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
	AL_CALL(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
	ALfloat forwardAndUpVectors[] = {
		1.0f, 0.f, 0.f, // forward
		0.f, 1.0f, 0.f // up
	};
	AL_CALL(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

    soundState.IsInitialized = true;
}

SoundSource SoundLoad(const char* path)
{
    SoundSource result = {
        .Buffer = 0,
        .Source = 0,
        .Valid = false
    };

    FileResult file = FileLoad(path);
	if (!file.Valid) 
    {
		printf("Failed to load sound %s!\n", path);
		return result;
	}

    WAVRiffHeader* header = (WAVRiffHeader*)file.Data;
    assert(memcmp(header->Riff, "RIFF", 4) == 0 && memcmp(header->Wave, "WAVE", 4) == 0);

    u8* currentPtr = (u8*)(header + 1);
    WAVFormatHeader* formatInfo = 0;
    void* sampleData = 0;
    u32 sampleDataSize = 0;
    while (currentPtr < file.Data + file.DataLength)
    {
        WAVChunkHeader* chunkHeader = (WAVChunkHeader*)currentPtr;

        if (memcmp(chunkHeader->Id, "fmt ", 4) == 0)
        {
            formatInfo = (WAVFormatHeader*)(currentPtr + sizeof(WAVChunkHeader));
        }
        else if (memcmp(chunkHeader->Id, "data", 4) == 0)
        {
            sampleDataSize = chunkHeader->Size;
            sampleData = currentPtr + sizeof(WAVChunkHeader);
        }

        currentPtr += sizeof(WAVChunkHeader) + chunkHeader->Size;
    }

    ALenum format = -1;
    if (formatInfo->BitsPerSample == 8) {
        if (formatInfo->NumChannels == 1) {
            format = AL_FORMAT_MONO8;
        }
        else if(formatInfo->NumChannels == 2) {
            format = AL_FORMAT_STEREO8;
        }
    }
    else if (formatInfo->BitsPerSample == 16) {
        if (formatInfo->NumChannels == 1) {
            format = AL_FORMAT_MONO16;
        }
        else if(formatInfo->NumChannels == 2) {
            format = AL_FORMAT_STEREO16;
        }
    }

    assert(format != -1 && "Unsupported audio format!\n");

	AL_CALL(alGenBuffers(1, &result.Buffer));
	AL_CALL(alBufferData(result.Buffer, format, sampleData, sampleDataSize, formatInfo->SampleRate));

	AL_CALL(alGenSources(1, &result.Source));
	AL_CALL(alSource3f(result.Source, AL_POSITION, 1.f, 0.f, 0.f));
	AL_CALL(alSource3f(result.Source, AL_VELOCITY, 0.f, 0.f, 0.f));
	AL_CALL(alSourcef(result.Source, AL_PITCH, 1.f));
	AL_CALL(alSourcef(result.Source, AL_GAIN, 1.f));
	AL_CALL(alSourcei(result.Source, AL_LOOPING, AL_FALSE));
	AL_CALL(alSourcei(result.Source, AL_BUFFER, result.Buffer));

    result.Valid = true;
    free(file.Data);

    return result;
}

void SoundPlay(SoundSource* sound)
{
    if (!soundState.IsInitialized)
    {
        return;
    }

    assert(sound);
    assert(sound->Valid);

    AL_CALL(alSourcePlay(sound->Source));
}

void SoundSetLoop(SoundSource* sound, boolean loop)
{
    if (!soundState.IsInitialized)
    {
        return;
    }

    assert(sound);
    assert(sound->Valid);
    AL_CALL(alSourcei(sound->Source, AL_LOOPING, loop));
}
