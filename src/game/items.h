#include "engine/engine.h"

#ifndef ITEMS_H
#define ITEMS_H

enum ItemId
{
    ItemId_None,

    ItemId_Sword,
    ItemId_Spear,

    ItemId_COUNT
};
typedef u32 ItemId;

enum ItemKind 
{
    ItemKind_None,

    ItemKind_Weapon,
    
    ItemKind_COUNT
};
typedef u32 ItemKind;

typedef struct ItemData
{
    ItemId Id;
    ItemKind Kind;
    Texture* Icon;
    Texture* Texture;
} ItemData;

void SetupItems();
ItemData* ItemDataById(ItemId id);


#endif