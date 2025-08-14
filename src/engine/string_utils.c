#include "string_utils.h"

boolean StringEndswith(const char* str, const char* postfix)
{
    assert(str);
    assert(postfix);

    u32 strLength = strlen(str);
    u32 postfixLength = strlen(postfix);

    if (strLength < postfixLength)
    {
        return false;
    }

    u32 postfixIdx = 0;
    for (s32 i = strLength - postfixLength; i < strLength; i++)
    {
        if (str[i] != postfix[postfixIdx++])
        {
            return false;
        }
    }

    return true;
}

boolean StringEquals(const char* strA, const char* strB)
{
    assert(strA);
    assert(strB);

    u32 strALength = strlen(strA);
    u32 strBLength = strlen(strB);

    if (strALength != strBLength)
    {
        return false;
    }

    if (strA == strB)
    {
        return true;
    }

    for (s32 i = 0; i < strALength; i++)
    {
        if (strA[i] != strB[i])
        {
            return false;
        }
    }

    return true;
}
