#include "opengl_util.h"

VertexArray VertexArrayCreate(
    VertexBufferLayoutElement* layout, u32 count,
    void* bufferData, u32 bufferByteSize, GLuint bufferUsage)
{
    assert(layout);
    VertexArray result = {};

    GLClearErrors();
    glGenVertexArrays(1, &result.Id);
    glBindVertexArray(result.Id);

    glGenBuffers(1, &result.BufferId);
    glBindBuffer(GL_ARRAY_BUFFER, result.BufferId);
    glBufferData(GL_ARRAY_BUFFER, bufferByteSize, bufferData, bufferUsage);

    result.BufferUsage = bufferUsage;

    u32 stride = 0;
    for (int i = 0; i < count; i++)
    {
        VertexBufferLayoutElement* elementLayout = layout + i;
        stride += elementLayout->ByteSize * elementLayout->ElementCount;
    }
    
	u32 offset = 0;
	u32 index = 0;
    for (int i = 0; i < count; i++)
    {
        glEnableVertexAttribArray(index);

        VertexBufferLayoutElement* elementLayout = layout + i;
        if (elementLayout->Type == GL_INT || elementLayout->Type == GL_UNSIGNED_INT) 
		{
			glVertexAttribIPointer(index, elementLayout->ElementCount, elementLayout->Type, stride, (void*)offset);
		}
		else 
		{
            glVertexAttribPointer(index, elementLayout->ElementCount, elementLayout->Type, GL_FALSE, stride, (void*)offset);
		}

        offset += elementLayout->ByteSize * elementLayout->ElementCount;
        index++;
    }

    GLAssertNoErrors("Creating vertex array");

    return result;
}

void VertexArrayAddIndexBuffer(VertexArray* vertexArray, u32* data, u32 length)
{
    assert(vertexArray);
    assert(data);

    GLClearErrors();

    glGenBuffers(1, &vertexArray->ElementBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArray->ElementBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * length, data, GL_STATIC_DRAW);

    GLAssertNoErrors("Adding index buffer to vertex array");
}

void VertexArrayBind(VertexArray* vertexArray)
{
    GLCall(glBindVertexArray(vertexArray->Id));
}

void VertexArrayUnbind()
{
    GLCall(glBindVertexArray(0));
}

void VertexArrayDestroy(VertexArray* vertexArray)
{
	GLCall(glDeleteBuffers(1, &vertexArray->BufferId));
	GLCall(glDeleteBuffers(1, &vertexArray->ElementBufferId);)
	GLCall(glDeleteVertexArrays(1, &vertexArray->Id));
}

void VertexArrayBufferSubData(VertexArray* vertexArray, void* data, u32 byteSize)
{
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, byteSize, data));
}

// ShaderProgram
u32 ShaderProgramGetUniformLocation(ShaderProgram* shader, const char* name)
{
    assert(shader);
    u32 nameLength = strlen(name) + 1;
    assert(nameLength < SHADER_UNIFORM_NAME_MAX_LEN);

    for (s32 i = 0; i < shader->uniformLocationCount; i++)
    {
        ShaderUniformLocation* location = shader->uniformLocations + i;
        if (strcmp(location->Name, name) == 0)
        {
            return location->Location;
        }
    }
    
    assert(shader->uniformLocationCount < SHADER_UNIFORM_COUNT_MAX - 1);
    GLClearErrors();

    u32 location = glGetUniformLocation(shader->Id, name);

    GLAssertNoErrors("Getting unform location in shader");

    ShaderUniformLocation* uniformLocation = shader->uniformLocations + shader->uniformLocationCount;
    shader->uniformLocationCount++;

    strcpy(uniformLocation->Name, name);
    uniformLocation->Location = location;

    return location;
}

void ShaderProgramBind(ShaderProgram* shader)
{
    assert(shader);
    GLCall(glUseProgram(shader->Id));
}

void ShaderProgramUnbind()
{
    GLCall(glUseProgram(0));
}

GLuint ShaderProgramCreateFromSource(
    const char* vertexSource,
    const char* fragmentSource
)
{
    GLClearErrors();

    GLuint vertexId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexId, 1, &vertexSource, NULL);
    glCompileShader(vertexId);

    int success = 0;
    char infoLog[512];
    glGetShaderiv(vertexId, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexId, 512, NULL, infoLog);
        printf("ERROR: Shader compilation failed\n%s\n", infoLog);
        assert(false);
    }

    GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentId, 1, &fragmentSource, NULL);
    glCompileShader(fragmentId);

    glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentId, 512, NULL, infoLog);
        printf("ERROR: Shader compilation failed\n%s\n", infoLog);
        assert(false);
    }

    GLuint shaderId = glCreateProgram();
    glAttachShader(shaderId, vertexId);
    glAttachShader(shaderId, fragmentId);
    glLinkProgram(shaderId);

    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderId, 512, NULL, infoLog);
        printf("ERROR: Program linking failed\n%s\n", infoLog);
        assert(false);
    }

    glUseProgram(shaderId);

    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);

    GLAssertNoErrors("Creating shader from source");

    return shaderId;
}

void ShaderDestroy(ShaderProgram* shader)
{
    GLCall(glDeleteShader(shader->Id));
}

void ShaderUniformMat4(ShaderProgram* shader, const char* name, f32* values)
{
    assert(shader);
    assert(name);
    assert(values);

    u32 location = ShaderProgramGetUniformLocation(shader, name);
    GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, values));
}

void ShaderUniformVec2(ShaderProgram* shader, const char* name, f32 x, f32 y)
{
    assert(shader);
    assert(name);

    u32 location = ShaderProgramGetUniformLocation(shader, name);
    GLCall(glUniform2f(location, x, y));
}

void ShaderUniformInts(ShaderProgram* shader, const char* name, s32* values, u32 count)
{
	GLuint location = ShaderProgramGetUniformLocation(shader, name);
	GLCall(glUniform1iv(location, count, values));
}

// Texture
Texture TextureLoad(const char* path)
{
    GLClearErrors();

    Texture result = {};
    int32_t channels = 0;
    unsigned char* data = stbi_load(path, &result.Width, &result.Height, &channels, 0);
    if (!data)
    {
		printf("Failed to load image '%s'\n", path);
		assert(false);
    }

    glGenTextures(1, &result.Id);
    glBindTexture(GL_TEXTURE_2D, result.Id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.Width, result.Height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    GLAssertNoErrors("Creating texture");

    return result;
}

void TextureBindId(GLuint id, u32 offset)
{
    GLCall(glActiveTexture(GL_TEXTURE0 + offset));
    GLCall(glBindTexture(GL_TEXTURE_2D, id));
}

void TextureBind(Texture* texture, u32 offset)
{
    assert(texture);

    GLCall(glActiveTexture(GL_TEXTURE0 + offset));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture->Id));
}

void TextureUnbind(u32 offset)
{
    GLCall(glActiveTexture(GL_TEXTURE0 + offset));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void TextureDestroy(Texture* texture)
{
    assert(texture);

    GLCall(glDeleteTextures(1, &texture->Id));
}
