#include "Game.h"

#include <cmath>
#include <string>

#include "raylib.h"

namespace
{
    const Color Background = {14, 15, 20, 255};
    const Color HudBackground = {10, 11, 15, 255};
    const Color BoardColor = {24, 26, 32, 255};
    const Color BoardColorAlt = {30, 33, 40, 255};
    const Color BorderColor = {55, 60, 72, 255};
    const Color OverlayBackground = {6, 7, 10, 215};
    const Color ScoreText = {130, 235, 140, 255};
    const Color BestText = {240, 200, 110, 255};
    const Color TitleText = {130, 235, 140, 255};
    const Color PlainText = {232, 234, 240, 255};
    const Color MutedText = {145, 150, 162, 255};
    const Color DangerText = {235, 90, 90, 255};
}

Game::Game()
    : state(GameState::Menu)
    , moveTimer(0.0f)
    , newRecord(false)
{
    food.Spawn(snake.GetBody());
}

Game::~Game() = default;

void Game::HandleInput()
{
    switch (state)
    {
        case GameState::Menu:
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R))
            {
                audio.PlayClick();
                StartRun();
            }
            break;
        }

        case GameState::Playing:
        {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            {
                snake.QueueDirection(Direction::Up);
            }
            else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            {
                snake.QueueDirection(Direction::Down);
            }
            else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            {
                snake.QueueDirection(Direction::Left);
            }
            else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            {
                snake.QueueDirection(Direction::Right);
            }

            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE))
            {
                audio.PlayClick();
                moveTimer = 0.0f;
                state = GameState::Paused;
            }
            break;
        }

        case GameState::Paused:
        {
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER))
            {
                audio.PlayClick();
                state = GameState::Playing;
            }
            else if (IsKeyPressed(KEY_R))
            {
                audio.PlayClick();
                StartRun();
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                audio.PlayClick();
                GoToMenu();
            }
            break;
        }

        case GameState::GameOver:
        {
            if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                audio.PlayClick();
                StartRun();
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                audio.PlayClick();
                GoToMenu();
            }
            break;
        }
    }
}

void Game::Update()
{
    if (state != GameState::Playing)
    {
        return;
    }

    moveTimer += GetFrameTime();

    const float interval = GetMoveInterval();

    while (moveTimer >= interval)
    {
        moveTimer -= interval;
        MoveOnce();

        if (state != GameState::Playing)
        {
            break;
        }
    }
}

void Game::Draw()
{
    BeginDrawing();

    ClearBackground(Background);
    DrawGrid();
    food.Draw();
    snake.Draw();
    DrawHud();

    if (state == GameState::Menu)
    {
        DrawMenuOverlay();
    }
    else if (state == GameState::Paused)
    {
        DrawPauseOverlay();
    }
    else if (state == GameState::GameOver)
    {
        DrawGameOverOverlay();
    }

    EndDrawing();
}

void Game::StartRun()
{
    snake.Reset();
    food.Spawn(snake.GetBody());
    score.ResetRun();
    moveTimer = 0.0f;
    newRecord = false;
    state = GameState::Playing;
}

void Game::GoToMenu()
{
    snake.Reset();
    food.Spawn(snake.GetBody());
    score.ResetRun();
    moveTimer = 0.0f;
    newRecord = false;
    state = GameState::Menu;
}

void Game::HandleGameOver()
{
    state = GameState::GameOver;
    newRecord = score.FinishRun();
    moveTimer = 0.0f;
    audio.PlayGameOver();
}

void Game::MoveOnce()
{
    const Position nextHead = snake.PredictHead();

    if (nextHead.x < 0 || nextHead.x >= Config::GridWidth || nextHead.y < 0 || nextHead.y >= Config::GridHeight)
    {
        HandleGameOver();
        return;
    }

    const bool ate = (nextHead == food.GetPosition());

    snake.Step(ate);

    if (ate)
    {
        score.AddFood();
        audio.PlayEat();
        food.Spawn(snake.GetBody());
    }

    if (snake.HitsItself())
    {
        HandleGameOver();
    }
}

float Game::GetMoveInterval() const
{
    const float interval = Config::StartMoveInterval - score.GetCurrent() * Config::ScoreSpeedFactor;

    return interval < Config::MinMoveInterval ? Config::MinMoveInterval : interval;
}

void Game::DrawGrid() const
{
    for (int row = 0; row < Config::GridHeight; row++)
    {
        for (int column = 0; column < Config::GridWidth; column++)
        {
            const Color cellColor = (row + column) % 2 == 0 ? BoardColor : BoardColorAlt;

            DrawRectangle(column * Config::CellSize,
                          Config::HudHeight + row * Config::CellSize,
                          Config::CellSize,
                          Config::CellSize,
                          cellColor);
        }
    }

    DrawRectangleLines(0,
                       Config::HudHeight,
                       Config::WindowWidth,
                       Config::GridHeight * Config::CellSize,
                       BorderColor);
}

void Game::DrawHud() const
{
    DrawRectangle(0, 0, Config::WindowWidth, Config::HudHeight, HudBackground);
    DrawRectangle(0, Config::HudHeight - 2, Config::WindowWidth, 2, BorderColor);

    const std::string scoreText = "SCORE  " + std::to_string(score.GetCurrent());
    const std::string bestText = "HIGH  " + std::to_string(score.GetHigh());
    const std::string lengthText = "LENGTH  " + std::to_string(snake.GetLength());

    DrawText(scoreText.c_str(), 24, 20, 24, ScoreText);
    DrawText(bestText.c_str(), Config::WindowWidth - MeasureText(bestText.c_str(), 24) - 24, 20, 24, BestText);
    DrawText(lengthText.c_str(), (Config::WindowWidth - MeasureText(lengthText.c_str(), 20)) / 2, 22, 20, MutedText);
}

void Game::DrawMenuOverlay() const
{
    DrawRectangle(0, Config::HudHeight, Config::WindowWidth,
                  Config::GridHeight * Config::CellSize, OverlayBackground);

    const std::string title = "SNAKE";
    DrawText(title.c_str(), (Config::WindowWidth - MeasureText(title.c_str(), 80)) / 2, 130, 80, TitleText);

    const std::string subtitle = "A C++ / RAYLIB ARCADE CLONE";
    DrawText(subtitle.c_str(), (Config::WindowWidth - MeasureText(subtitle.c_str(), 20)) / 2, 225, 20, MutedText);

    if (std::fmod(GetTime(), 1.0) < 0.6)
    {
        const std::string prompt = "PRESS ENTER TO START";
        DrawText(prompt.c_str(), (Config::WindowWidth - MeasureText(prompt.c_str(), 28)) / 2, 300, 28, PlainText);
    }

    const std::string controlsTitle = "CONTROLS";
    DrawText(controlsTitle.c_str(), (Config::WindowWidth - MeasureText(controlsTitle.c_str(), 22)) / 2, 380, 22, BestText);

    const std::string moveLine = "ARROWS / WASD   MOVE";
    const std::string pauseLine = "P / ESC         PAUSE";
    const std::string restartLine = "R               PLAY AGAIN";

    DrawText(moveLine.c_str(), (Config::WindowWidth - MeasureText(moveLine.c_str(), 20)) / 2, 418, 20, PlainText);
    DrawText(pauseLine.c_str(), (Config::WindowWidth - MeasureText(pauseLine.c_str(), 20)) / 2, 448, 20, PlainText);
    DrawText(restartLine.c_str(), (Config::WindowWidth - MeasureText(restartLine.c_str(), 20)) / 2, 478, 20, PlainText);
}

void Game::DrawPauseOverlay() const
{
    DrawRectangle(0, Config::HudHeight, Config::WindowWidth,
                  Config::GridHeight * Config::CellSize, OverlayBackground);

    const std::string title = "PAUSED";
    DrawText(title.c_str(), (Config::WindowWidth - MeasureText(title.c_str(), 64)) / 2, 210, 64, TitleText);

    const std::string resumeLine = "P / ENTER   RESUME";
    const std::string restartLine = "R          RESTART";
    const std::string menuLine = "ESC        MAIN MENU";

    DrawText(resumeLine.c_str(), (Config::WindowWidth - MeasureText(resumeLine.c_str(), 24)) / 2, 330, 24, PlainText);
    DrawText(restartLine.c_str(), (Config::WindowWidth - MeasureText(restartLine.c_str(), 24)) / 2, 366, 24, PlainText);
    DrawText(menuLine.c_str(), (Config::WindowWidth - MeasureText(menuLine.c_str(), 24)) / 2, 402, 24, PlainText);
}

void Game::DrawGameOverOverlay() const
{
    DrawRectangle(0, Config::HudHeight, Config::WindowWidth,
                  Config::GridHeight * Config::CellSize, OverlayBackground);

    const std::string title = "GAME OVER";
    DrawText(title.c_str(), (Config::WindowWidth - MeasureText(title.c_str(), 64)) / 2, 160, 64, DangerText);

    const std::string scoreLine = "SCORE  " + std::to_string(score.GetCurrent());
    DrawText(scoreLine.c_str(), (Config::WindowWidth - MeasureText(scoreLine.c_str(), 32)) / 2, 260, 32, PlainText);

    if (newRecord)
    {
        const std::string recordLine = "NEW BEST SCORE!";
        DrawText(recordLine.c_str(), (Config::WindowWidth - MeasureText(recordLine.c_str(), 26)) / 2, 315, 26, BestText);
    }
    else
    {
        const std::string bestLine = "HIGH  " + std::to_string(score.GetHigh());
        DrawText(bestLine.c_str(), (Config::WindowWidth - MeasureText(bestLine.c_str(), 26)) / 2, 315, 26, BestText);
    }

    const std::string restartLine = "R / ENTER   PLAY AGAIN";
    const std::string menuLine = "ESC         MAIN MENU";

    DrawText(restartLine.c_str(), (Config::WindowWidth - MeasureText(restartLine.c_str(), 24)) / 2, 410, 24, PlainText);
    DrawText(menuLine.c_str(), (Config::WindowWidth - MeasureText(menuLine.c_str(), 24)) / 2, 446, 24, MutedText);
}
