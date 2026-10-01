/**
 * @file Food.h
 * @brief Collectible food: random placement that never lands on the snake.
 */
#pragma once

#include <random>
#include <vector>

#include "Types.h"

/**
 * @brief The single piece of food on the board.
 *
 * Spawns by rejection sampling: cells are drawn uniformly at random until one
 * is free, which guarantees food is never placed underneath the snake.
 */
class Food
{
public:
    /** @brief Seeds the generator from std::random_device so runs differ. */
    Food();

    /**
     * @brief Picks a new free cell for the food.
     * @param occupiedCells Every cell the snake currently covers; candidates
     *        matching one of them are rejected. The call returns immediately if
     *        the snake already fills the whole board.
     */
    void Spawn(const std::vector<Position>& occupiedCells);

    /** @brief Draws the food as a filled circle with a small highlight. */
    void Draw() const;

    /** @return The grid cell the food currently occupies. */
    Position GetPosition() const;

private:
    /**
     * @brief Whether @p candidate is covered by @p occupiedCells.
     *
     * A linear scan is fine here: the board holds at most 600 cells and this
     * runs once per bite, not per frame.
     *
     * @param candidate      Cell to test.
     * @param occupiedCells  Cells already taken by the snake.
     * @return true when the candidate is occupied.
     */
    bool IsOccupied(const Position& candidate, const std::vector<Position>& occupiedCells) const;

    Position position;       /**< Cell the food currently sits on. */
    std::mt19937 rng;        /**< Per-instance generator (not shared between foods). */
};
