/**
 * @file Game.h
 * @brief Game state machine: input handling, simulation timing, collisions, and rendering.
 */
#pragma once

#include "Audio.h"
#include "Food.h"
#include "Score.h"
#include "Snake.h"
#include "Types.h"
#include "raylib.h" // RenderTexture2D member below

/**
 * @brief Ties the entities together into a playable game.
 *
 * One instance lives for the whole lifetime of the window. Each frame it reads
 * input for the current state, advances the simulation only while Playing, and
 * draws the board plus the overlay that matches the current state.
 */
class Game
{
public:
    /** @brief Starts on the menu with an initial piece of food already placed. */
    Game();

    /**
     * @brief Unloads the baked board texture.
     *
     * Runs while the window (and therefore the GPU context) still exists —
     * main.cpp destroys the Game inside an inner scope, before CloseWindow().
     */
    ~Game();

    /**
     * @brief Reads the keyboard for the current state and performs any transition.
     *
     * Only ever consumes one state's keys, so bindings cannot be ambiguous
     * between menu, playing, paused, and game over.
     */
    void HandleInput();

    /**
     * @brief Advances the simulation while Playing; ignores everything else.
     *
     * Accumulates real frame time and then applies one or more fixed-step moves,
     * keeping the snake's speed independent of the render frame rate (and of
     * frame hitches, which are caught up instead of skipped).
     */
    void Update();

    /** @brief Draws the board, entities, HUD, and the overlay for the current state. */
    void Draw();

private:
    /**
     * @brief Restores a fresh board (snake, food, score, timers) and parks the
     *        state machine on @p next.
     * @param next State to enter: GameState::Playing to start, GameState::Menu to idle.
     */
    void ResetBoard(GameState next);

    /** @brief Full reset into GameState::Playing. */
    void StartRun();

    /** @brief Full reset into GameState::Menu. */
    void GoToMenu();

    /** @brief Handles death: commits the high score if beaten, plays the jingle, and enters GameState::GameOver. */
    void HandleGameOver();

    /**
     * @brief Applies one logical move and resolves everything it can trigger.
     *
     * Order matters and is deliberate:
     *  1. Predict the head and test it against the grid — death is decided
     *     before any mutation, so a fatal move never lands on the board.
     *  2. Step the snake (which pops one buffered turn).
     *  3. Test food, then self-collision. Because Step() removes the tail
     *     first, moving into the cell the tail just vacated is legal.
     */
    void MoveOnce();

    /**
     * @brief Paints the static checkerboard and border into boardTexture once.
     *
     * The grid never changes, so baking it at startup replaces 600
     * DrawRectangle() calls per frame with a single texture blit.
     * Called from the constructor; unloaded by the destructor.
     */
    void BuildBoardTexture();

    /**
     * @brief Seconds between moves at the current score.
     * @return Config::StartMoveInterval reduced by the score, clamped at
     *         Config::MinMoveInterval so the game stays playable at high scores.
     */
    float GetMoveInterval() const;

    // --- Rendering helpers; each assumes BeginDrawing() has already been called ---
    void DrawGrid() const;            /**< Checkerboard play area and border. */
    void DrawHud() const;             /**< Score, high score, and length strip. */
    void DrawMenuOverlay() const;     /**< Title, controls, and blinking start prompt. */
    void DrawPauseOverlay() const;    /**< Pause card and its keys. */
    void DrawGameOverOverlay() const; /**< Final score, possible "new best", and keys. */
    void DrawDebugOverlay() const;    /**< Teaching overlay (toggled with F1): entities, queue, timer. */

    Snake snake;   /**< Player-controlled entity. */
    Food food;     /**< Collectible; respawns on every bite. */
    Score score;   /**< Run score and persisted best. */
    Audio audio;   /**< Sound effects; non-copyable, which makes Game non-copyable. */

    GameState state;      /**< Current state machine node. */
    float moveTimer;      /**< Seconds accumulated toward the next move. */
    bool newRecord;       /**< Set when FinishRun() beat the previous high score. */
    bool showDebug;       /**< Whether the F1 teaching overlay is visible. */
    RenderTexture2D boardTexture; /**< Pre-rendered grid, built once in the constructor. */
};
