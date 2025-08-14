#include "opengl_util.h"
#include "engine_math.h"

#ifndef RENDER_H
#define RENDER_H

typedef struct QuadDrawCmd
{
    Vec2 Position;
    Vec2 Size;
    Vec3 Color;
    Vec2 UvMin;
    Vec2 UvMax;
    Texture Texture;
    u32 TextureIndex;
    float ColorOverwrite;
    float Alpha;
    boolean FlipTextureX;
    s32 ZLayer;
    f32 Rotation;
} QuadDrawCmd;

typedef struct AnimationData
{
    f32 SecondsPerFrame;
    u32 TextureFrameCount;
    u32 FrameIndexStart;
    u32 FrameIndexEnd;
    Texture* Texture;
} AnimationData;

typedef struct Animation
{
    AnimationData* Data;
    f32 Timer;
    boolean Playing;
} Animation;

boolean RenderInit(u32 quadCapacity);
void RenderDestroy();
void RenderSetProjection(Mat4 mat);

void UpdateAnimation(Animation* animation, f32 delta);

QuadDrawCmd* DrawQuad(Vec2 bottomLeft, Vec2 size, Vec3 color);
QuadDrawCmd* DrawTexture(Vec2 bottomLeft, Texture* texture);
QuadDrawCmd* DrawAnimation(Vec2 bottomLeft, Animation* animation);

void RenderStartFrame();
void RenderEndFrame();

#endif
