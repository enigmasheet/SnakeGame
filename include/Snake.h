#pragma once

#include <cstddef>
#include <deque>
#include <vector>

#include "Types.h"

class Snake
{
public:
    Snake();

    void Reset();
    void QueueDirection(Direction newDirection);
    Position PredictHead() const;
    void Step(bool grow);
    bool HitsWall() const;
    bool HitsItself() const;
    void Draw() const;

    const std::vector<Position>& GetBody() const;
    Direction GetDirection() const;
    std::size_t GetLength() const;

private:
    std::vector<Position> body;
    Direction direction;
    std::deque<Direction> pendingDirections;
};
