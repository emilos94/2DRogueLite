#include "engine_internal.h"
#include "assert.h"

#ifndef ARENA_H
#define ARENA_H

#define KB(x) (x * 1024)
#define MB(x) (KB(x) * 1024)

typedef struct Arena
{   
    u8* Buffer;
    u32 Capacity;
    u32 Offset;
} Arena;

Arena ArenaCreate(u32 capacity);
void* ArenaPushN(Arena* arena, u32 amount);

#define ArenaPush(arena, type) ((type*)ArenaPushN(arena, sizeof(type)))
#define ArenaPushArr(arena, type, count) ((type*)ArenaPushN(arena, sizeof(type) * count))

#endif 