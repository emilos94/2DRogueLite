#include "engine_internal.h"
#include "stdio.h"
#include "string_utils.h"
#include "datastructures.h"
#include "arena.h"
#include "render.h"

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

typedef struct FileResult
{
    u8* Data;
    u32 DataLength;
    boolean Valid;
} FileResult;

FileResult FileLoad(const char* path);

typedef struct KeyValuePair
{
    struct KeyValuePair* Next;
    struct KeyValuePair* Prev;
    String Key;
    String Value;
} KeyValuePair;

typedef struct KeyValueFile
{
    KeyValuePair* KeyValuePairFirst;
    u32 KeyValuePairCount;
    boolean Valid;
} KeyValueFile;

KeyValueFile FileLoadKeyValues(Arena* arena, const char* path, char delimiter);
boolean FileLoadFont(Arena* permanentArena, Arena scratchArena, Font* result, const char* path);



#endif