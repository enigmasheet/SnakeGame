#include "Game.h"
#include "Types.h"
#include "raylib.h"

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(Config::WindowWidth, Config::WindowHeight, "Snake - C++ and Raylib");
    SetTargetFPS(60);

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
