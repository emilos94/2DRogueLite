#include "engine/engine.h"

#ifndef ITEMS_H
#define ITEMS_H

enum ItemId
{
    ItemId_None,

    ItemId_Sword,

    ItemId_COUNT
};

enum ItemKind 
{
    ItemKind_None,

    ItemKind_Weapon,
    
    ItemKind_COUNT
};

typedef struct
{
    ItemId Id;
    ItemKind Kind;

    // Weapon
    

} Item;


#endif