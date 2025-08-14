#include "engine_internal.h"
#include "render.h"
#include "string_utils.h"
#include "sound.h"
#include "file_util.h"

#ifndef ENGINE_H
#define ENGINE_H

// Window
boolean WindowCreate(const char* title, u32 width, u32 height);
void WindowPollEvents();
void WindowSwapBuffers();
void WindowClear();
boolean WindowShouldClose();
void WindowDestroy();
u32 WindowWidth();
u32 WindowHeight();

// Input
boolean KeyPressed(u32 key);
boolean KeyDown(u32 key);
boolean KeyReleased(u32 key);

Vec2 MousePosition(void);
boolean MouseButtonPressed(u32 button);
boolean MouseButtonDown(u32 button);
boolean MouseButtonReleased(u32 button);
void InputClear(void);

#endif