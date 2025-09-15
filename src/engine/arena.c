#include "arena.h"

Arena ArenaCreate(u32 capacity)
{
    Arena result = {
        .Capacity = capacity,
        .Offset = 0,
        .Buffer = malloc(capacity)
    };

    assert(result.Buffer);
    memset(result.Buffer, 0, capacity);

    return result;
}

// Goldmine source for arenas: https://nullprogram.com/blog/2023/09/27/
void* ArenaPushN(Arena* arena, u32 amount)
{
    const u32 align = 8;
    uintptr_t currentPtr = (uintptr_t)arena->Buffer + arena->Offset;
    uintptr_t misAlignment = currentPtr & (align - 1);
    u32 padding = (align - misAlignment) & (align - 1);
    
    s32 available = arena->Capacity - (arena->Offset + padding);
    u32 alignedAmount = amount + padding;
    if (available <= 0 || alignedAmount > available)
    {
        assert(false);
    }

    char* ptr = arena->Buffer + arena->Offset;
    memset(ptr, 0, alignedAmount);
    arena->Offset += alignedAmount;
    return ptr;
}
