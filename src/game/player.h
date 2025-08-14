#include "game_internal.h"

#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

void PlayerInput(GameState* gameState, Entity* player, f32 delta);
void PlayerUpdate(GameState* gameState, Entity* player, f32 delta);

#endif