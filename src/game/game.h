#include "engine/engine.h"

#ifndef GAME_H
#define GAME_H

void GameInit();
void GameUpdate(float delta);
void GameRender(float delta);
boolean GameRunning();
void GameDestroy();

#endif