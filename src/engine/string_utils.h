#include "engine_internal.h"

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

typedef struct String
{
    char* Chars;
    u32 Length;
} String;

#define StringLit(x) ((String){ .Chars = x, .Length = strlen(x)})

boolean StringEndswith(const char* str, const char* postfix);
boolean CharsEquals(const char* strA, const char* strB);
boolean StringEquals(String a, String b);
boolean CharIsWhiteSpace(char c);

#endif