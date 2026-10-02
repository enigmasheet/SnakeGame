/**
 * @file Game.cpp
 * @brief Game core: lifetime, the state machine, fixed-step timing, and collisions.
 *
 * The input half of Game lives in GameInput.cpp and the drawing half in
 * GameRender.cpp. What is left here is the part that decides *what happens*:
 * constructing a fresh board, draining the move timer, and applying one
 * logical move in the order that makes death, growth, and self-collision
 * behave correctly.
 */

#include "Game.h"

#include "raylib.h"

Game::Game()
    : state(GameState::Menu)
    , moveTimer(0.0f)
    , newRecord(false)
    , showDebug(false)
{
    // The grid is static, so it is painted once here rather than every frame.
    BuildBoardTexture();

    // Place the first food immediately so the menu already shows a live board.
    food.Spawn(snake.GetBody());
}

Game::~Game()
{
    // Must happen before CloseWindow() destroys the GL context that owns it;
    // main.cpp guarantees that by declaring Game in an inner scope.
    UnloadRenderTexture(boardTexture);
}

void Game::Update()
{
    if (state != GameState::Playing)
    {
        return;
    }

    moveTimer += GetFrameTime();

    const float interval = GetMoveInterval();

    // Fixed-timestep accumulator: a slow frame may owe several moves, so drain
    // the timer instead of stepping at most once. Speed therefore depends only
    // on the score, never on the render frame rate.
    while (moveTimer >= interval)
    {
        moveTimer -= interval;
        MoveOnce();

        // A move can end the run — stop consuming the timer rather than
        // stepping a snake that is already dead.
        if (state != GameState::Playing)
        {
            break;
        }
    }
}

void Game::StartRun()
{
    ResetBoard(GameState::Playing);
}

void Game::GoToMenu()
{
    ResetBoard(GameState::Menu);
}

void Game::ResetBoard(GameState next)
{
    // One routine restores a fresh board; StartRun() and GoToMenu() differ only
    // in the state they park the machine on. Adding another "reset" screen means
    // one more one-line wrapper, not another copy of these five lines.
    snake.Reset();
    food.Spawn(snake.GetBody());
    score.ResetRun();
    moveTimer = 0.0f;
    newRecord = false;
    state = next;
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
    // 1. Test the *predicted* head: death is decided before any state changes,
    //    so a fatal move is never committed to the board.
    const Position nextHead = snake.PredictHead();

    if (!IsWithinGrid(nextHead))
    {
        HandleGameOver();
        return;
    }

    const bool ate = (nextHead == food.GetPosition());

    // 2. Step the snake (this also pops one buffered turn). `ate` decides
    //    whether the tail is kept.
    snake.Step(ate);

    if (ate)
    {
        // Score, sound cue, and a respawn that skips every current snake cell.
        score.AddFood();
        audio.PlayEat();
        food.Spawn(snake.GetBody());
    }

    // 3. Self-collision is tested last: Step() already removed the tail, so
    //    sliding into the cell the tail just vacated is correctly allowed.
    if (snake.HitsItself())
    {
        HandleGameOver();
    }
}

float Game::GetMoveInterval() const
{
    // The curve itself lives in Types.h as a free function so the tests can pin
    // its starting value and its floor without a window. See ComputeMoveInterval()
    // for the formula and the score at which it bottoms out.
    return ComputeMoveInterval(score.GetCurrent());
}
