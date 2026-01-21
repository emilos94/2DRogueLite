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

KeyValueFile FileLoadKeyValues(Arena* arena, const char* path, char delimiter)
{
    KeyValueFile result = {0};
    FileResult file = FileLoad(path);
    if (!file.Valid)
    {
        return result;
    }

    KeyValuePair* currentKVPair = ArenaPush(arena, KeyValuePair);
    result.KeyValuePairFirst = currentKVPair;
    result.KeyValuePairCount++;

    u32 marker = 0;
    for (s32 i = 0; i < file.DataLength; i++)
    {
        if (file.Data[i] == delimiter)
        {
            u32 length = i - marker;
            currentKVPair->Key.Chars = ArenaPushArr(arena, char, length);
            currentKVPair->Key.Length = length;
            memcpy(currentKVPair->Key.Chars, &file.Data[marker], length);
            marker = i+1;
        }
        else if (file.Data[i] == '"')
        {
            marker = i+1;
            i++;
            while(i < file.DataLength && file.Data[i] != '"') i++;
            
            u32 length = i - marker;
            currentKVPair->Value.Chars = ArenaPushArr(arena, char, length);
            currentKVPair->Value.Length = length;
            memcpy(currentKVPair->Value.Chars, &file.Data[marker], length);

            KeyValuePair* newKVPair = ArenaPush(arena, KeyValuePair);
            LinkedListAddBack(currentKVPair, newKVPair);
            currentKVPair = newKVPair;
            result.KeyValuePairCount++;
            
            i++; // skip "
            while(i < file.DataLength && CharIsWhiteSpace(file.Data[i])) i++;
            i--;
            marker = i+1;
        }
        else if (currentKVPair->Key.Chars != 0 && (CharIsWhiteSpace(file.Data[i]) || i == file.DataLength - 1))
        {
            u32 length = i - marker;
            currentKVPair->Value.Chars = ArenaPushArr(arena, char, length);
            currentKVPair->Value.Length = length;
            memcpy(currentKVPair->Value.Chars, &file.Data[marker], length);

            if (i < file.DataLength - 1)
            {
                KeyValuePair* newKVPair = ArenaPush(arena, KeyValuePair);
                LinkedListAddBack(currentKVPair, newKVPair);
                currentKVPair = newKVPair;
                result.KeyValuePairCount++;
                
                while(i < file.DataLength && CharIsWhiteSpace(file.Data[i])) i++;
                i--;
                marker = i+1;
            }
        }
    }

    free(file.Data);
    result.Valid = true;
    return result;
}

boolean FileLoadFont(Arena* permanentArena, Arena scratchArena, Font* font, const char* path)
{
    KeyValueFile kvFile = FileLoadKeyValues(&scratchArena, path, '=');
    if (!kvFile.Valid)
    {
        return false;
    }

    CharacterInfo* charInfoCurrent = 0;

    for (KeyValuePair* kv = kvFile.KeyValuePairFirst; kv; kv = kv->Next)
    {
        if (StringEquals(kv->Key, StringLit("info face")))
        {
            assert(kv->Value.Length <= FONT_FACE_MAX);
            memcpy(&font->Face, kv->Value.Chars, kv->Value.Length);
        }
        else if (StringEquals(kv->Key, StringLit("file")))
        {
            assert(kv->Value.Length <= FONT_FACE_MAX);
            memcpy(&font->TextureFile, kv->Value.Chars, kv->Value.Length);
        }
        else if (StringEquals(kv->Key, StringLit("padding")))
        {
            // parse padding
			u32 paddingIndex = 0;
			u32 marker = 0;
			for (s32 i = 0; i < kv->Value.Length; i++) {
				while (kv->Value.Chars[i] != ',') {
					i++;
				}

				font->Paddings[paddingIndex++] = atoi(kv->Value.Chars + marker);
				i++;
				marker = i;
			}
        }
        else if (StringEquals(kv->Key, StringLit("common lineHeight")))
        {
            font->LineHeight = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("base")))
        {
            font->Base = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("scaleW")))
        {
            font->Width = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("scaleH")))
        {
            font->Height = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("chars count")))
        {
            font->CharacterInfoCount = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("char id")))
        {
            if (charInfoCurrent)
            {
                CharacterInfo* newCharInfo = ArenaPush(permanentArena, CharacterInfo);
                LinkedListAddBack(charInfoCurrent, newCharInfo);
                charInfoCurrent = newCharInfo;
            }
            else
            {
                font->CharacterInfoFirst = ArenaPush(permanentArena, CharacterInfo);
                charInfoCurrent = font->CharacterInfoFirst;
            }

            charInfoCurrent->Id = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("x")))
        {
            charInfoCurrent->X = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("y")))
        {
            charInfoCurrent->Y = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("width")))
        {
            charInfoCurrent->Width = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("height")))
        {
            charInfoCurrent->Height = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("xoffset")))
        {
            charInfoCurrent->XOffset = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("yoffset")))
        {
            charInfoCurrent->YOffset = atoi(kv->Value.Chars);
        }
        else if (StringEquals(kv->Key, StringLit("xadvance")))
        {
            charInfoCurrent->XAdvance = atoi(kv->Value.Chars);
        }
    }

    return true;
}
