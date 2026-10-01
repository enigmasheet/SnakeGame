#pragma once

#include <random>
#include <vector>

#include "Types.h"

class Food
{
public:
    Food();

    void Spawn(const std::vector<Position>& occupiedCells);
    void Draw() const;

    Position GetPosition() const;

private:
    bool IsOccupied(const Position& candidate, const std::vector<Position>& occupiedCells) const;

    Position position;
    std::mt19937 rng;
};
