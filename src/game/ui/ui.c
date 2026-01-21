#include "ui.h"

f32 F32CenterIn(f32 containerStart, f32 containerEnd, f32 toCenterWidth)
{
    f32 result = (containerEnd - containerStart) / 2 - toCenterWidth / 2;
    return result;
}

boolean QuadHovered(QuadDrawCmd* quad)
{
    Vec2 mousePos = MousePosResolution();

    boolean hovering = 
        mousePos.x >= quad->Position.x && mousePos.x <= quad->Position.x + quad->Size.x &&
        mousePos.y >= quad->Position.y && mousePos.y <= quad->Position.y + quad->Size.y;

    if (hovering)
    {
        gameState.UIHovered = true;
    }

    return hovering;
}

void UIActionBarUpdate(f32 delta)
{
    if (gameState.GameMode != GameMode_Playing)
    {
        return;
    }

    for (s32 i = 0; i < ACTION_BAR_SLOTS; i++)
    {
        if (KeyPressed(GLFW_KEY_1 + i))
        {
            gameState.ActionBarSelectedIndex = i;
        }
    }
}

void UIActionBarRender(f32 delta)
{
    const u32 itemSlotCount = 10;

    f32 itemSlotPadding = 2;

    Texture* actionBarTexture = GetTexture("action_bar.png");
    QuadDrawCmd* actionBar = DrawTexture(V2(0, 2), actionBarTexture);
    actionBar->Position.x = F32CenterIn(0, RESOLUTION_WIDTH, actionBarTexture->Width);
    actionBar->ZLayer = ZLayer_UI0;

    f32 x0 = actionBar->Position.x + 6; // 6 pixels for proper spot in

    Texture* actionBarSlotTexture = GetTexture("action_bar_slot.png");

    for (s32 i = 0; i < itemSlotCount; i++)
    {
        QuadDrawCmd* slot = DrawTexture(V2(x0, actionBar->Position.y + 2), actionBarSlotTexture);
        slot->ZLayer = ZLayer_UI1;

        if (QuadHovered(slot))
        {
            if (MouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                gameState.ActionBarSelectedIndex = i;
            }

            // todo: implement drag/drop
        }

        if (i == gameState.ActionBarSelectedIndex)
        {
            QuadDrawCmd* selector = DrawTexture(Vec2Sub(slot->Position, V2(1, 1)), GetTexture("action_bar_selector.png"));
            selector->ZLayer = ZLayer_UI1;
        }

        ActionBarSlot* actionBarSlot = &gameState.ActionBarSlots[i];
        if (actionBarSlot->ItemId != ItemId_None)
        {
            ItemData* itemData = ItemDataById(actionBarSlot->ItemId);

            QuadDrawCmd* item = DrawTexture(V2(x0, actionBar->Position.y + 2), itemData->Icon);
            item->ZLayer = ZLayer_UI1;
        }
        
        x0 += actionBarSlotTexture->Width + itemSlotPadding;
    }
}

void UIActionBarSetItem(u32 index, ItemId id)
{
    assert(index < ACTION_BAR_SLOTS);

    gameState.ActionBarSlots[index] = (ActionBarSlot) {
        .Count = 1,
        .Index = index,
        .ItemId = id
    };
}