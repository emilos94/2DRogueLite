#include "assets.h"

typedef struct AssetState
{
    Texture* Textures;
    u32 TextureCount;

    AnimationData AnimationData[AnimationId_COUNT];
} AssetState;
AssetState assetState;

void LoadTextures(void)
{
    struct dirent *de;  // Pointer for directory entry
    const char* relativePath = "res/images/";
    u32 relativePathLength = strlen(relativePath);
    DIR *dr = opendir(relativePath);

    if (!dr) 
    {
        printf("Failed to load images!\n");
        assert(false);
    }
    char path[512] = {0};
    char* fileBuffer = 0;

    int numberOfImages = 0;
    while((de = readdir(dr)) != NULL) 
    {
        if (StringEndswith(de->d_name, ".png")) 
        {
            numberOfImages++;
        }
    }
    closedir(dr);
    
    assetState.TextureCount = numberOfImages;
    assetState.Textures = malloc(assetState.TextureCount * sizeof(Texture));

    // Reopen to reset offset
    dr = opendir(relativePath);
    s32 idx = 0;
    while((de = readdir(dr)) != NULL) 
    {
        if (StringEndswith(de->d_name, ".png")) 
        {
            memcpy(path, relativePath, relativePathLength);
            memcpy(path + relativePathLength, de->d_name, de->d_namlen);

            assert(de->d_namlen < TEXTURE_NAME_CAPACITY);
            
            assetState.Textures[idx] = TextureLoad(path);
            
            Texture* texture = assetState.Textures + idx;
            memcpy(texture->Name, de->d_name, de->d_namlen);
           
            memset(path, 0, 512);
            idx++;
        }
    }
    closedir(dr);
}

void SetupAnimationData(void)
{
    assetState.AnimationData[AnimationId_PlayerIdle] = (AnimationData) {
        .TextureFrameCount = 6,
        .FrameIndexEnd = 1,
        .FrameIndexStart = 0,
        .SecondsPerFrame = 0.4,
        .Texture = GetTexture("player.png")
    };
    
    assetState.AnimationData[AnimationId_PlayerRun] = (AnimationData) {
        .TextureFrameCount = 6,
        .FrameIndexEnd = 5,
        .FrameIndexStart = 2,
        .SecondsPerFrame = 0.15,
        .Texture = GetTexture("player.png")
    };
    
    assetState.AnimationData[AnimationId_EffectSlash] = (AnimationData) {
        .TextureFrameCount = 4,
        .FrameIndexStart = 0,
        .FrameIndexEnd = 3,
        .SecondsPerFrame = 0.1,
        .Texture = GetTexture("slash_effect_32.png")
    };
    
    assetState.AnimationData[AnimationId_EffectPoof] = (AnimationData) {
        .TextureFrameCount = 4,
        .FrameIndexStart = 0,
        .FrameIndexEnd = 3,
        .SecondsPerFrame = 0.1,
        .Texture = GetTexture("poof_effect.png")
    };
        
    assetState.AnimationData[AnimationId_EffectGroundImpact] = (AnimationData) {
        .TextureFrameCount = 5,
        .FrameIndexStart = 0,
        .FrameIndexEnd = 4,
        .SecondsPerFrame = 0.1,
        .Texture = GetTexture("ground_impact_effect.png")
    };
}

void AssetsLoad(void)
{
    LoadTextures();
    SetupAnimationData();
}

void AssetsDestroy(void)
{
    for (s32 i = 0; i < assetState.TextureCount; i++)
    {
        TextureDestroy(assetState.Textures + i);
    }

    free(assetState.Textures);
}

Texture* GetTexture(const char* path)
{
    assert(path);

    for (s32 i = 0; i < assetState.TextureCount; i++)
    {
        Texture* texture = assetState.Textures + i;
        // note: if str eq ever becomes a problem, precompute hash ?
        // Probably wont, only use this on entity creation
        if (StringEquals(path, texture->Name))
        {
            return texture;
        }
    }

    assert(false);
}

AnimationData* GetAnimation(AnimationId id)
{
    assert(id < AnimationId_COUNT);

    return &assetState.AnimationData[id];
}
