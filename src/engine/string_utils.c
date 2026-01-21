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


boolean _CharsEquals(char* strA, u32 strALength, char* strB, u32 strBLength)
{
    assert(strA);
    assert(strB);

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

boolean CharsEquals(const char* strA, const char* strB)
{
    return _CharsEquals(strA, strlen(strA), strB, strlen(strB));
}

boolean StringEquals(String a, String b)
{
    return _CharsEquals(a.Chars, a.Length, b.Chars, b.Length);
}

boolean CharIsWhiteSpace(char c)
{
    boolean result = c == ' ' || c == '\n' || c == '\r' || c == '\t';
    return result;
}
