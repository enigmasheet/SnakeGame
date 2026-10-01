/**
 * @file main.cpp
 * @brief Entry point: window setup, the per-frame loop, and teardown order.
 */

#include "Game.h"
#include "Types.h"
#include "raylib.h"

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);

    // LOG_WARNING suppresses raylib's per-frame info logging. Set before
    // InitWindow so it takes effect for window creation too.
    SetTraceLogLevel(LOG_WARNING);

    InitWindow(Config::WindowWidth, Config::WindowHeight, "Snake - C++ and Raylib");
    SetTargetFPS(60);

    // raylib treats ESC as the exit key by default, which would make
    // WindowShouldClose() return true the moment the game used it to pause —
    // every documented ESC action would quit the program. Retire ESC as the
    // exit key so only the window close button (or Alt+F4) ends the run.
    SetExitKey(KEY_NULL);

    // Deliberate inner scope: Game (and its Audio member) must be destroyed
    // while the window is still alive, so sounds unload against a working
    // context instead of after CloseWindow().
    {
        Game game;

        while (!WindowShouldClose())
        {
            game.HandleInput();
            game.Update();
            game.Draw();
        }
    }

    CloseWindow();

    return 0;
}
