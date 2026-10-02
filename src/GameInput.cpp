/**
 * @file GameInput.cpp
 * @brief Game input: one keyboard branch per state, plus the F1 teaching overlay.
 *
 * The reading half of Game lives here. HandleInput() consults only the keys
 * belonging to the *current* state, so a binding can never mean two different
 * things at once (arrows steer while Playing, do nothing while Paused).
 */

#include "Game.h"

#include "raylib.h"

void Game::HandleInput()
{
    // The teaching overlay toggles in every state, so it can explain the menu
    // and the game-over screen too.
    if (IsKeyPressed(KEY_F1))
    {
        showDebug = !showDebug;
    }

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
                moveTimer = 0.0f; // zeroed so resuming cannot fire a move instantly
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
