/**
 * @file Game.cpp
 * @brief Implementation of Game: input dispatch, fixed-step timing, collisions,
 *        and all HUD/overlay rendering.
 */

#include "Game.h"

#include <cmath>
#include <string>

#include "raylib.h"

// Dark arcade palette, kept in one place so the whole theme can be retinted here.
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

    // --- File-local helpers ---------------------------------------------------

    /**
     * Draws @p text horizontally centred in the window.
     *
     * MeasureText() returns the rendered pixel width for a given font size, so
     * the left edge is (window width - text width) / 2. Centralising it here
     * replaced the same expression that used to be written out at every call.
     *
     * @param text  NUL-terminated string to draw.
     * @param y     Top pixel of the text.
     * @param size  Font size in pixels.
     * @param color Fill colour.
     */
    void DrawCenteredText(const char* text, int y, int size, Color color)
    {
        DrawText(text, (Config::WindowWidth - MeasureText(text, size)) / 2, y, size, color);
    }

    /** @return Short display name for a direction (used by the F1 overlay). */
    const char* DirectionName(Direction direction)
    {
        switch (direction)
        {
            case Direction::Up:    return "Up";
            case Direction::Down:  return "Down";
            case Direction::Left:  return "Left";
            case Direction::Right: return "Right";
        }
        return "?";
    }

    /** @return Short display name for a game state (used by the F1 overlay). */
    const char* StateName(GameState state)
    {
        switch (state)
        {
            case GameState::Menu:     return "Menu";
            case GameState::Playing:  return "Playing";
            case GameState::Paused:   return "Paused";
            case GameState::GameOver: return "GameOver";
        }
        return "?";
    }
}

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

    // Drawn last so the read-out stays visible on top of the dim overlays.
    if (showDebug)
    {
        DrawDebugOverlay();
    }

    EndDrawing();
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

void Game::BuildBoardTexture()
{
    // Paint the checkerboard and border into a texture exactly once. The board
    // never changes, so DrawGrid() can then blit it instead of issuing
    // GridWidth * GridHeight DrawRectangle() calls every frame.
    boardTexture = LoadRenderTexture(Config::WindowWidth, Config::GridHeight * Config::CellSize);

    BeginTextureMode(boardTexture);
    ClearBackground(Background);

    for (int row = 0; row < Config::GridHeight; row++)
    {
        for (int column = 0; column < Config::GridWidth; column++)
        {
            const Color cellColor = (row + column) % 2 == 0 ? BoardColor : BoardColorAlt;

            DrawRectangle(column * Config::CellSize,
                          row * Config::CellSize,
                          Config::CellSize,
                          Config::CellSize,
                          cellColor);
        }
    }

    DrawRectangleLines(0,
                       0,
                       Config::WindowWidth,
                       Config::GridHeight * Config::CellSize,
                       BorderColor);
    EndTextureMode();
}

void Game::DrawGrid() const
{
    // Framebuffers are bottom-up in OpenGL, so the source rectangle uses a
    // negative height to flip the texture the right way up while copying it.
    const Rectangle source{0.0f,
                           0.0f,
                           static_cast<float>(Config::WindowWidth),
                           -static_cast<float>(Config::GridHeight * Config::CellSize)};
    const Vector2 dest{0.0f, static_cast<float>(Config::HudHeight)};

    DrawTextureRec(boardTexture.texture, source, dest, WHITE);
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
    DrawCenteredText(lengthText.c_str(), 22, 20, MutedText);
}

void Game::DrawMenuOverlay() const
{
    DrawRectangle(0, Config::HudHeight, Config::WindowWidth,
                  Config::GridHeight * Config::CellSize, OverlayBackground);

    DrawCenteredText("SNAKE", 130, 80, TitleText);
    DrawCenteredText("A C++ / RAYLIB ARCADE CLONE", 225, 20, MutedText);

    // Blinking prompt: visible for 0.6 s of every 1 s cycle.
    if (std::fmod(GetTime(), 1.0) < 0.6)
    {
        DrawCenteredText("PRESS ENTER TO START", 300, 28, PlainText);
    }

    DrawCenteredText("CONTROLS", 380, 22, BestText);
    DrawCenteredText("ARROWS / WASD   MOVE", 418, 20, PlainText);
    DrawCenteredText("P / ESC         PAUSE", 448, 20, PlainText);
    DrawCenteredText("R               PLAY", 478, 20, PlainText);
    DrawCenteredText("F1              DEBUG OVERLAY", 508, 20, MutedText);
}

void Game::DrawPauseOverlay() const
{
    DrawRectangle(0, Config::HudHeight, Config::WindowWidth,
                  Config::GridHeight * Config::CellSize, OverlayBackground);

    DrawCenteredText("PAUSED", 210, 64, TitleText);
    DrawCenteredText("P / ENTER   RESUME", 330, 24, PlainText);
    DrawCenteredText("R          RESTART", 366, 24, PlainText);
    DrawCenteredText("ESC        MAIN MENU", 402, 24, PlainText);
}

void Game::DrawGameOverOverlay() const
{
    DrawRectangle(0, Config::HudHeight, Config::WindowWidth,
                  Config::GridHeight * Config::CellSize, OverlayBackground);

    DrawCenteredText("GAME OVER", 160, 64, DangerText);

    const std::string scoreLine = "SCORE  " + std::to_string(score.GetCurrent());
    DrawCenteredText(scoreLine.c_str(), 260, 32, PlainText);

    if (newRecord)
    {
        DrawCenteredText("NEW BEST SCORE!", 315, 26, BestText);
    }
    else
    {
        const std::string bestLine = "HIGH  " + std::to_string(score.GetHigh());
        DrawCenteredText(bestLine.c_str(), 315, 26, BestText);
    }

    DrawCenteredText("R / ENTER   PLAY AGAIN", 410, 24, PlainText);
    DrawCenteredText("ESC         MAIN MENU", 446, 24, MutedText);
}

void Game::DrawDebugOverlay() const
{
    const int panelX = 8;
    const int panelWidth = 360;
    const int panelHeight = 116;
    const int panelY = Config::WindowHeight - panelHeight - 8;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Color{6, 7, 10, 235});
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, BorderColor);

    const Position& head = snake.GetBody().front();
    const Position& tail = snake.GetBody().back();

    // Render the buffered turns as a list so the queue is visible while playing.
    std::string queue = "[";

    const std::deque<Direction>& pending = snake.GetPendingDirections();

    for (std::size_t i = 0; i < pending.size(); i++)
    {
        if (i > 0)
        {
            queue += ", ";
        }
        queue += DirectionName(pending[i]);
    }

    queue += "]";

    // Each line formats and draws in one expression: raylib's TextFormat() reuses
    // a small ring of static buffers, so two results must not be held at once.
    const int lineX = panelX + 10;
    int lineY = panelY + 8;

    DrawText(TextFormat("F1 HIDE    FPS %d    STATE %s", GetFPS(), StateName(state)),
             lineX, lineY, 16, PlainText);
    lineY += 20;

    DrawText(TextFormat("HEAD %d,%d    TAIL %d,%d    LENGTH %d",
                        head.x, head.y, tail.x, tail.y, static_cast<int>(snake.GetLength())),
             lineX, lineY, 16, MutedText);
    lineY += 20;

    DrawText(TextFormat("QUEUED TURNS %s", queue.c_str()), lineX, lineY, 16, BestText);
    lineY += 20;

    DrawText(TextFormat("MOVE TIMER %.3f / %.3f s", moveTimer, GetMoveInterval()),
             lineX, lineY, 16, ScoreText);
    lineY += 20;

    DrawText(TextFormat("GRID %d x %d    CELL %d px", Config::GridWidth, Config::GridHeight, Config::CellSize),
             lineX, lineY, 16, MutedText);
}
