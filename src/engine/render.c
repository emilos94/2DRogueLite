#include "render.h"

// todo: query ?
#define QUAD_RENDER_2D_TEXTURE_CAPACITY 16

typedef struct QuadVertex
{
    Vec2 Position;
    Vec2 UvCoordinate;
    Vec3 Color;
    u32 TextureIndex;
    f32 ColorOverwrite;
    f32 Alpha;
    f32 Rotation;
    Vec2 CenterPoint;
} QuadVertex;

typedef struct RenderState
{
    VertexArray VertexArray;
    ShaderProgram Shader;

    QuadVertex* Vertices;
    u32 QuadCount;

    QuadDrawCmd* DrawCommands;
    u32 QuadCapacity;

    u32 TextureCount;
    u32 TextureIndices[16];
} RenderState;

RenderState renderState = {};

boolean RenderInit(u32 quadCapacity)
{
    renderState.QuadCapacity = quadCapacity;
    renderState.QuadCount = 0;

    renderState.Vertices = malloc(quadCapacity * 4 * sizeof(QuadVertex));
    memset(renderState.Vertices, 0, quadCapacity * 4 * sizeof(QuadVertex));

    renderState.DrawCommands = malloc(quadCapacity * sizeof(QuadDrawCmd));
    memset(renderState.DrawCommands, 0, quadCapacity * sizeof(QuadDrawCmd));

    VertexBufferLayoutElement layout[] = {
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 2 },        // Position
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 2 },        // Uv
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 3 },        // Color
        { .Type = GL_UNSIGNED_INT, .ByteSize = sizeof(u32), .ElementCount = 1 }, // TextureIndex
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 1 },        // ColorOverwrite
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 1 },        // Alpha
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 1 },        // Rotation
        { .Type = GL_FLOAT, .ByteSize = sizeof(f32), .ElementCount = 2 },        // CenterPoint
    };

    renderState.VertexArray = VertexArrayCreate(
        layout, 8, renderState.Vertices, quadCapacity * 4 * sizeof(QuadVertex), GL_DYNAMIC_DRAW
    );
    u32* indices = malloc(quadCapacity * 6 * sizeof(u32));
	u32 value = 0;
	for (s32 i = 0; i < quadCapacity * 6; i += 6) {
		indices[i] = value + 0;
		indices[i + 1] = value + 1;
		indices[i + 2] = value + 2;
		indices[i + 3] = value + 2;
		indices[i + 4] = value + 3;
		indices[i + 5] = value + 0;
		value += 4;
	}
    VertexArrayAddIndexBuffer(&renderState.VertexArray, indices, quadCapacity * 6);
    free(indices);

    const char* vertexSource = 
    "#version 330 core\n"
    "layout (location=0) in vec2 pos;\n"
    "layout (location=1) in vec2 uv;\n"
    "layout (location=2) in vec3 color;\n"
    "layout (location=3) in int textureIndex;\n"
    "layout (location=4) in float colorOverwrite;\n"
    "layout (location=5) in float alpha;\n"
    "layout (location=6) in float rotation;\n"
    "layout (location=7) in vec2 centerPoint;\n"
    "uniform mat4 projection;\n"
    "uniform vec2 cameraPosition;\n"
    "out vec3 vColor;\n"
    "out vec2 vUv;\n"
    "flat out int vTextureIndex;\n"
    "out float vColorOverwrite;\n"
    "out float vAlpha;\n"
    "void main() {\n"
        "vColor = color;\n"
        "vUv = uv;\n"
        "vTextureIndex = textureIndex;\n"
        "vColorOverwrite = colorOverwrite;\n"
        "vAlpha = alpha;\n"
        "float rotationRadians = radians(rotation);\n"
        "vec2 pointAtOrigin = pos - centerPoint;\n"
        "vec2 rotatedPoint = vec2(pointAtOrigin.x * cos(rotationRadians) - pointAtOrigin.y * sin(rotationRadians), pointAtOrigin.x * sin(rotationRadians) + pointAtOrigin.y * cos(rotationRadians)) + centerPoint;\n"
        "rotatedPoint -= cameraPosition;\n"
        "gl_Position = projection * vec4(rotatedPoint.x, rotatedPoint.y, 0.0, 1.0);\n"
    "}\n";
    
    const char* fragmentSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 vColor;\n"
    "in vec2 vUv;\n"
    "flat in int vTextureIndex;\n"
    "in float vColorOverwrite;\n"
    "in float vAlpha;\n"
    "uniform sampler2D textures[16];\n"
    "const int max_texture_index = 15;\n"
    "void main() {\n"
        "vec4 textureOut = vec4(1.0,1.0,1.0,1.0);\n"
        "if (vTextureIndex <= max_texture_index) {\n"
            "textureOut = texture(textures[vTextureIndex], vUv);\n"
        "}\n"
        "vec3 color = mix(textureOut.xyz, vColor.xyz, vColorOverwrite);\n"
        "FragColor = vec4(color, min(textureOut.w, vAlpha));\n"
    "}\n";

    renderState.Shader.Id = ShaderProgramCreateFromSource(vertexSource, fragmentSource);
    
    renderState.TextureCount = 0;
    s32 samplers[QUAD_RENDER_2D_TEXTURE_CAPACITY];
	for (s32 i = 0; i < QUAD_RENDER_2D_TEXTURE_CAPACITY; i++) {
		samplers[i] = i;
	}
	ShaderUniformInts(&renderState.Shader, "textures", samplers, QUAD_RENDER_2D_TEXTURE_CAPACITY);

    stbi_set_flip_vertically_on_load(true);
    
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCall(glEnable(GL_BLEND));
}

void RenderSetProjection(Mat4 mat)
{
    Mat4 columnMajorMat = Mat4ColumnMajor(mat);
    ShaderUniformMat4(&renderState.Shader, "projection", columnMajorMat.m);
}

void RenderSetCameraPos(Vec2 position)
{
    ShaderUniformVec2(&renderState.Shader, "cameraPosition", position.x, position.y);
}

void RenderDestroy()
{
    free(renderState.Vertices);
    free(renderState.DrawCommands);

    VertexArrayDestroy(&renderState.VertexArray);
    ShaderDestroy(&renderState.Shader);
}

void RenderStartFrame()
{
    renderState.QuadCount = 0;
    VertexArrayBind(&renderState.VertexArray);
    ShaderProgramBind(&renderState.Shader);
}

int QuadDrawCmdCompare(const void* a, const void* b)  {
    QuadDrawCmd* left = (QuadDrawCmd*)a;
    QuadDrawCmd* right = (QuadDrawCmd*)b;

    u32 left_z = left->ZLayer;
    u32 right_z = right->ZLayer;

    if (left_z > right_z) {
        return 1;
    }
    else if (left_z < right_z) {
        return -1;
    }

    // If z equals, sort by y
    u32 left_y = left->Position.y;
    u32 right_y = right->Position.y;
    if (left_y < right_y) {
        return 1;
    }
    else if (left_z > right_z) {
        return -1;
    }

    return 0;
}

void Flush()
{
    if (renderState.QuadCount == 0)
    {
        return;
    }

    qsort(renderState.DrawCommands, renderState.QuadCount, sizeof(QuadDrawCmd), QuadDrawCmdCompare);

    for (int i = 0; i < renderState.QuadCount; i++)
    {
        uint32_t vertexOffset = i * 4;
        QuadDrawCmd* cmd = renderState.DrawCommands + i;

        f32 uvXMin = cmd->FlipTextureX ? cmd->UvMax.x : cmd->UvMin.x;
        f32 uvXMax = cmd->FlipTextureX ? cmd->UvMin.x : cmd->UvMax.x;

        Vec2 quadCenterPoint = (Vec2) {.x = cmd->Position.x + cmd->Size.x / 2.0, .y = cmd->Position.y + cmd->Size.y / 2.0 };

        // tl
        renderState.Vertices[vertexOffset + 0].Position.x = cmd->Position.x;
        renderState.Vertices[vertexOffset + 0].Position.y = cmd->Position.y + cmd->Size.y;
        renderState.Vertices[vertexOffset + 0].UvCoordinate.x = uvXMin;
        renderState.Vertices[vertexOffset + 0].UvCoordinate.y = cmd->UvMax.y;
        renderState.Vertices[vertexOffset + 0].Color = cmd->Color;
        renderState.Vertices[vertexOffset + 0].Alpha = cmd->Alpha;
        renderState.Vertices[vertexOffset + 0].TextureIndex = cmd->TextureIndex;
        renderState.Vertices[vertexOffset + 0].ColorOverwrite = cmd->ColorOverwrite;
        renderState.Vertices[vertexOffset + 0].Rotation = cmd->Rotation;
        renderState.Vertices[vertexOffset + 0].CenterPoint = quadCenterPoint;

        // bl
        renderState.Vertices[vertexOffset + 1].Position.x = cmd->Position.x;
        renderState.Vertices[vertexOffset + 1].Position.y = cmd->Position.y;
        renderState.Vertices[vertexOffset + 1].UvCoordinate.x = uvXMin;
        renderState.Vertices[vertexOffset + 1].UvCoordinate.y = cmd->UvMin.y;
        renderState.Vertices[vertexOffset + 1].Color = cmd->Color;
        renderState.Vertices[vertexOffset + 1].Alpha = cmd->Alpha;
        renderState.Vertices[vertexOffset + 1].TextureIndex = cmd->TextureIndex;
        renderState.Vertices[vertexOffset + 1].ColorOverwrite = cmd->ColorOverwrite;
        renderState.Vertices[vertexOffset + 1].Rotation = cmd->Rotation;
        renderState.Vertices[vertexOffset + 1].CenterPoint = quadCenterPoint;

        // br
        renderState.Vertices[vertexOffset + 2].Position.x = cmd->Position.x + cmd->Size.x;
        renderState.Vertices[vertexOffset + 2].Position.y = cmd->Position.y;
        renderState.Vertices[vertexOffset + 2].UvCoordinate.x = uvXMax;
        renderState.Vertices[vertexOffset + 2].UvCoordinate.y = cmd->UvMin.y;
        renderState.Vertices[vertexOffset + 2].Color = cmd->Color;
        renderState.Vertices[vertexOffset + 2].Alpha = cmd->Alpha;
        renderState.Vertices[vertexOffset + 2].TextureIndex = cmd->TextureIndex;
        renderState.Vertices[vertexOffset + 2].ColorOverwrite = cmd->ColorOverwrite;
        renderState.Vertices[vertexOffset + 2].Rotation = cmd->Rotation;
        renderState.Vertices[vertexOffset + 2].CenterPoint = quadCenterPoint;

        // tr
        renderState.Vertices[vertexOffset + 3].Position.x = cmd->Position.x + cmd->Size.x;
        renderState.Vertices[vertexOffset + 3].Position.y = cmd->Position.y + cmd->Size.y;
        renderState.Vertices[vertexOffset + 3].UvCoordinate.x = uvXMax;
        renderState.Vertices[vertexOffset + 3].UvCoordinate.y = cmd->UvMax.y;
        renderState.Vertices[vertexOffset + 3].Color = cmd->Color;
        renderState.Vertices[vertexOffset + 3].Alpha = cmd->Alpha;
        renderState.Vertices[vertexOffset + 3].TextureIndex = cmd->TextureIndex;
        renderState.Vertices[vertexOffset + 3].ColorOverwrite = cmd->ColorOverwrite;
        renderState.Vertices[vertexOffset + 3].Rotation = cmd->Rotation;
        renderState.Vertices[vertexOffset + 3].CenterPoint = quadCenterPoint;
    }

    VertexArrayBufferSubData(&renderState.VertexArray, renderState.Vertices, sizeof(QuadVertex) * renderState.QuadCount * 4);

    for (s32 i = 0; i < renderState.TextureCount; i++) 
    {
        TextureBindId(renderState.TextureIndices[i], i);
    }

    GLCall(glDrawElements(GL_TRIANGLES, renderState.QuadCount * 6, GL_UNSIGNED_INT, NULL));

    for (s32 i = 0; i < renderState.TextureCount; i++) 
    {
        TextureUnbind(i);
    }

    renderState.QuadCount = 0;
    renderState.TextureCount = 0;
}

void RenderEndFrame()
{
    Flush();

    VertexArrayUnbind();
    ShaderProgramUnbind();
}

void AnimationUpdate(Animation* animation, f32 delta)
{
    assert(animation);
    if (!animation->Playing)
    {
        animation->JustFinished = false;
        return;
    }

    animation->Timer += delta;
    if (animation->Timer >= AnimationDuration(animation))
    {
        animation->Timer = 0;
        animation->JustFinished = true;

        if (!animation->ShouldLoop)
        {
            animation->Playing = false;
        }
    }
}

f32 AnimationDuration(Animation* animation)
{
    assert(animation);

    f32 result = animation->Data->SecondsPerFrame * (f32)(animation->Data->FrameIndexEnd - animation->Data->FrameIndexStart + 1);
    return result;
}

u32 AnimationFrameWidth(Animation* animation)
{
    assert(animation);

    u32 frameCount = (animation->Data->FrameIndexEnd - animation->Data->FrameIndexStart + 1);
    u32 result = animation->Data->Texture->Width / frameCount;
    return result;
}

QuadDrawCmd* DrawAnimation(Vec2 bottomLeft, Animation* animation)
{
    assert(animation);
    AnimationData* data = animation->Data;

    QuadDrawCmd* drawCmd = DrawTexture(bottomLeft, data->Texture);

    drawCmd->Size.x = data->Texture->Width / data->TextureFrameCount;
    
    f32 uvxPerFrame = 1.0 / (f32)data->TextureFrameCount;
    
    f32 animationFrameCount = (f32)(data->FrameIndexEnd - data->FrameIndexStart) + 1;
    f32 duration = data->SecondsPerFrame * animationFrameCount;
    f32 currentFrame = floorf((animation->Timer / duration) * animationFrameCount);

    f32 uvx0 = uvxPerFrame * (currentFrame + data->FrameIndexStart);
    f32 uvx1 = uvx0 + uvxPerFrame;

    drawCmd->UvMin.x = uvx0;
    drawCmd->UvMax.x = uvx1;

    return drawCmd;
}

QuadDrawCmd* DrawQuad(Vec2 bottomLeft, Vec2 size, Vec3 color)
{
    if (renderState.QuadCount == renderState.QuadCapacity - 1)
    {
        Flush();
    }

    QuadDrawCmd* cmd = renderState.DrawCommands + renderState.QuadCount;
    cmd->Position = bottomLeft;
    cmd->Size = size;
    cmd->Color = color;
    cmd->UvMin = (Vec2){0, 0};
    cmd->UvMax = (Vec2){1, 1};
    cmd->TextureIndex = QUAD_RENDER_2D_TEXTURE_CAPACITY;
    cmd->ColorOverwrite = 1.0;
    cmd->Alpha = 1.0;
    cmd->Rotation = 0.0;
    cmd->FlipTextureX = false;

    renderState.QuadCount++;

    return cmd;
}


QuadDrawCmd* DrawTexture(Vec2 bottomLeft, Texture* texture)
{
    if (renderState.QuadCount == renderState.QuadCapacity - 1)
    {
        Flush();
    }
    
    s32 textureIndex = -1;
    for (s32 i = 0; i < renderState.TextureCount; i++)
    {
        if (renderState.TextureIndices[i] == texture->Id)
        {
            textureIndex = i;
            break;
        }
    }

    boolean textureNotRegistered = textureIndex < 0;
    boolean textureCapacityExceeded = renderState.TextureCount == QUAD_RENDER_2D_TEXTURE_CAPACITY - 1;
    if (textureNotRegistered)
    {
        if (textureCapacityExceeded)
        {
            Flush();
        }

        renderState.TextureIndices[renderState.TextureCount] = texture->Id;
        textureIndex = renderState.TextureCount;
        renderState.TextureCount++;
    }
    
    QuadDrawCmd* cmd = renderState.DrawCommands + renderState.QuadCount;
    cmd->Position = bottomLeft;
    cmd->Size = (Vec2) { texture->Width, texture->Height };
    cmd->Color = (Vec3){1, 1, 1};
    cmd->UvMin = (Vec2){0, 0};
    cmd->UvMax = (Vec2){1, 1};

    cmd->TextureIndex = textureIndex;
    cmd->ColorOverwrite = 0.0;
    cmd->Alpha = 1.0;
    cmd->Rotation = 0.0;
    cmd->FlipTextureX = false;

    renderState.QuadCount++;

    return cmd;
}
