#include "engine_internal.h"
#include "stdio.h"

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

typedef struct FileResult
{
    u8* Data;
    u32 DataLength;
    boolean Valid;
} FileResult;

FileResult FileLoad(const char* path);

#endif