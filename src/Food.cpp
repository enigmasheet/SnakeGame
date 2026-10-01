/**
 * @file Food.cpp
 * @brief Implementation of Food: rejection-sampled spawning and circle rendering.
 */

#include "Food.h"

#include <algorithm>

#include "raylib.h"

namespace
{
    const Color FoodColor = {232, 76, 76, 255};
    const Color FoodHighlight = {255, 170, 170, 255};
}

Food::Food()
    : position{0, 0}
    , rng(std::random_device{}())
{
}

void Food::Spawn(const std::vector<Position>& occupiedCells)
{
    const int totalCells = Config::GridWidth * Config::GridHeight;

    // Nothing to choose from if the snake covers every cell; keep the old spot
    // rather than looping forever.
    if (static_cast<int>(occupiedCells.size()) >= totalCells)
    {
        return;
    }

    std::uniform_int_distribution<int> columnDistribution(0, Config::GridWidth - 1);
    std::uniform_int_distribution<int> rowDistribution(0, Config::GridHeight - 1);

    Position candidate = position;

    // Rejection sampling: redraw until the cell is free. Every cell is equally
    // likely, and with a snake covering only a few dozen of 600 cells the loop
    // exits after one or two tries.
    do
    {
        candidate.x = columnDistribution(rng);
        candidate.y = rowDistribution(rng);
    }
    while (IsOccupied(candidate, occupiedCells));

    position = candidate;
}

void Food::Draw() const
{
    const float centerX = static_cast<float>(position.x * Config::CellSize) + Config::CellSize / 2.0f;
    const float centerY = static_cast<float>(Config::HudHeight + position.y * Config::CellSize) + Config::CellSize / 2.0f;

    DrawCircleV({centerX, centerY}, Config::CellSize / 2.0f - 4.0f, FoodColor);
    DrawCircleV({centerX - 3.0f, centerY - 3.0f}, 3.0f, FoodHighlight);
}

Position Food::GetPosition() const
{
    return position;
}

bool Food::IsOccupied(const Position& candidate, const std::vector<Position>& occupiedCells) const
{
    return std::find(occupiedCells.begin(), occupiedCells.end(), candidate) != occupiedCells.end();
}
