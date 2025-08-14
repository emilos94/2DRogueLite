#ifndef OPENGL_UTIL_H
#define OPENGL_UTIL_H

#include "engine_internal.h"

#define GLAssertNoErrors(blockName) assert(GLLogCall(blockName, __FILE__, __LINE__))

#define GLCall(x) GLClearErrors();\
	x;\
	assert(GLLogCall(#x, __FILE__, __LINE__));

#define GLScope(name, x) GLClearErrors();x;GLAssertNoErrors(name)

static void GLClearErrors()
{
	while (glGetError() != GL_NO_ERROR);
}

static boolean GLLogCall(const char* functionName, const char* fileName, int line)
{
	boolean noErrors = true;
	GLenum error = glGetError();
	while (error)
	{
		printf("[OpenGL ERROR]: (%d) %s %s %d\n", error, functionName, fileName, line);
		noErrors = false;
		GLenum error = glGetError();
	}

	return noErrors;
}

// Vertex array
typedef struct VertexBufferLayoutElement
{
    GLuint Type;
	u32 ElementCount;
	u32 ByteSize;

} VertexBufferLayoutElement;

typedef struct VertexArray
{
    GLuint Id;
    GLuint BufferId;
    GLuint ElementBufferId;
    GLuint BufferUsage;
} VertexArray;

VertexArray VertexArrayCreate(
    VertexBufferLayoutElement* layout, u32 count,
    void* bufferData, u32 bufferByteSize, GLuint bufferUsage
);
void VertexArrayAddIndexBuffer(VertexArray* vertexArray, u32* data, u32 length);
void VertexArrayBind(VertexArray* vertexArray);
void VertexArrayUnbind(void);
void VertexArrayDestroy(VertexArray* vertexArray);
void VertexArrayBufferSubData(VertexArray* vertexArray, void* data, u32 byteSize);

// Shaderprogram
#define SHADER_UNIFORM_NAME_MAX_LEN 32
typedef struct ShaderUniformLocation
{
    const char Name[SHADER_UNIFORM_NAME_MAX_LEN];
    u32 Location;
} ShaderUniformLocation;

#define SHADER_UNIFORM_COUNT_MAX 10
typedef struct ShaderProgram
{
    GLuint Id;
    ShaderUniformLocation uniformLocations[SHADER_UNIFORM_COUNT_MAX];
    u32 uniformLocationCount;
} ShaderProgram;

GLuint ShaderProgramCreateFromSource(
    const char* vertexSource,
    const char* fragmentSource
);

u32 ShaderProgramGetUniformLocation(ShaderProgram* shader, const char* name);
void ShaderProgramBind(ShaderProgram* shader);
void ShaderProgramUnbind();
void ShaderDestroy(ShaderProgram* shader);
void ShaderUniformMat4(ShaderProgram* shader, const char* name, f32* values);
void ShaderUniformInts(ShaderProgram* shader, const char* name, s32* values, u32 count);

// Texture
#define TEXTURE_NAME_CAPACITY 64
typedef struct Texture
{
    char Name[TEXTURE_NAME_CAPACITY];
    GLuint Id;
    u32 Width;
    u32 Height;
} Texture;

Texture TextureLoad(const char* path);
void TextureBindId(GLuint id, u32 offset);
void TextureBind(Texture* texture, u32 offset);
void TextureUnbind(u32 offset);
void TextureDestroy(Texture* texture);

#endif