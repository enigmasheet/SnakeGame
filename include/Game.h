#pragma once

#include "Audio.h"
#include "Food.h"
#include "Score.h"
#include "Snake.h"
#include "Types.h"

class Game
{
public:
    Game();
    ~Game();

    void HandleInput();
    void Update();
    void Draw();

private:
    void StartRun();
    void RestartRun();
    void GoToMenu();
    void HandleGameOver();
    void MoveOnce();
    float GetMoveInterval() const;

    void DrawGrid() const;
    void DrawHud() const;
    void DrawMenuOverlay() const;
    void DrawPauseOverlay() const;
    void DrawGameOverOverlay() const;

    Snake snake;
    Food food;
    Score score;
    Audio audio;

    GameState state;
    float moveTimer;
    bool newRecord;
};
