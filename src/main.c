#include "game/game.h"

int main(void)
{
    GameInit();

    f32 targetFps = 60.0;
    f32 secondsPerUpdate = 1.0 / targetFps;
    f32 lastTime = glfwGetTime();
    f32 accumulator = 0.0;
    f32 secondTimer = 0.0;
    s32 fps = 0;

    while(!WindowShouldClose() && GameRunning())
    {
        f32 now = glfwGetTime();
        f32 elapsed = now - lastTime;
        lastTime = now;

        accumulator += elapsed;
        secondTimer += elapsed;
        
        if (accumulator >= secondsPerUpdate)
        {
            WindowClear();
            
            InputClear();
            WindowPollEvents();
            GameUpdate(accumulator);
            
            RenderStartFrame();
            GameRender(accumulator);
            RenderEndFrame();

            WindowSwapBuffers();
            
            accumulator = 0.0;
            fps++;
        }

        if (secondTimer >= 1.0)
        {
            printf("[FPS]: %d\n", fps);
            fps = 0;
            secondTimer = 0.0;
        }
    }

    GameDestroy();
    RenderDestroy();
    WindowDestroy();
}
