/**
 * @file Snake.h
 * @brief Snake entity: body segments, buffered turns, movement, and collision queries.
 */
#pragma once

#include <cstddef>
#include <deque>
#include <vector>

#include "Types.h"

/**
 * @brief The player's snake, stored as a chain of grid cells.
 *
 * The body is a std::vector whose front is always the head. Turns are buffered
 * in a small queue so key presses arriving between two steps are still honoured,
 * while a 180 degree reversal (which would run the head into its own neck) is
 * impossible to queue.
 */
class Snake
{
public:
    /** @brief Builds a snake and immediately calls Reset(). */
    Snake();

    /**
     * @brief Restores the starting position, length, and direction.
     *
     * Places a horizontal run of Config::InitialSnakeLength segments on the
     * middle row facing Right, and discards any buffered turns.
     */
    void Reset();

    /**
     * @brief Requests a turn for an upcoming step.
     *
     * The request is dropped when it would reverse or repeat the last queued
     * direction, and ignored outright once two turns are already buffered —
     * that cap keeps prediction (PredictHead) and application (Step) in sync
     * without an unbounded queue.
     *
     * @param newDirection Requested direction; silently ignored when invalid.
     */
    void QueueDirection(Direction newDirection);

    /**
     * @brief Cell the head will occupy after the next Step(), without moving anything.
     *
     * Peeks at pendingDirections without consuming it, so callers can test the
     * result (wall collision) before mutating state. Step() pops that same entry,
     * so the two stay consistent.
     *
     * @return Predicted head cell, one step ahead of GetBody().front().
     */
    Position PredictHead() const;

    /**
     * @brief Advances the snake one cell, applying one buffered turn if present.
     *
     * Inserts a new head at the front, then removes the tail unless growing.
     *
     * @param grow true to keep the tail (food was eaten), false to slide the
     *        whole body forward.
     */
    void Step(bool grow);

    /**
     * @brief Whether the current head is outside the play grid.
     * @return true if the head cell is out of bounds.
     * @note During play the wall check runs against PredictHead() instead, so
     *       death is detected before the snake mutates; this helper tests the
     *       head as it currently stands and is used by the tests.
     */
    bool HitsWall() const;

    /**
     * @brief Whether the head overlaps any other segment.
     * @return true when a segment at index 1 or later shares the head's cell.
     *         The loop starts at 1 to skip the head itself.
     */
    bool HitsItself() const;

    /** @brief Renders the body as a colour gradient plus the head with eyes. */
    void Draw() const;

    /** @brief Read-only view of the segments, head first. The reference stays valid until the snake changes. */
    const std::vector<Position>& GetBody() const;

    /** @brief Direction actually travelled by the last Step() (buffer applied). */
    Direction GetDirection() const;

    /** @brief Segment count, always >= Config::InitialSnakeLength. */
    std::size_t GetLength() const;

private:
    std::vector<Position> body;              /**< Segments; front() is the head. */
    Direction direction;                     /**< Direction applied by the most recent Step(). */
    std::deque<Direction> pendingDirections; /**< Buffered turns, at most 2, consumed front-first. */
};
