#include "engine/engine.h"
#include "../game_internal.h"

#ifndef UI_H
#define UI_H

void UIActionBarUpdate(f32 delta);
void UIActionBarRender(f32 delta);
void UIActionBarSetItem(u32 index, ItemId id);

#endif