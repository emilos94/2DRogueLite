#include "file_util.h"

FileResult FileLoad(const char* path)
{
    FileResult result = {
        .Data = 0,
        .DataLength = 0,
        .Valid = false
    };

    FILE* fp = fopen(path, "rb");
    if (!fp)
    {
        printf("[ERROR] Failed to load file %s\n", path);
        return result;
    }

    fseek(fp, 0L, SEEK_END);
    u32 size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    u8* bufferPtr = malloc(size);
    result.Data = bufferPtr;

    u32 totalRead = fread(bufferPtr, sizeof(u8), size, fp);

    if (fclose(fp) == EOF)
    {
        printf("[ERROR] Failed to close file %s\n", path);
    }

    result.DataLength = totalRead;
    result.Valid = true;
    return result;
}
