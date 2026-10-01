#pragma once

struct Position
{
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const
    {
        return !(*this == other);
    }
};

enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

enum class GameState
{
    Menu,
    Playing,
    Paused,
    GameOver
};

namespace Config
{
    inline constexpr int CellSize = 25;
    inline constexpr int GridWidth = 30;
    inline constexpr int GridHeight = 20;
    inline constexpr int HudHeight = 60;
    inline constexpr int WindowWidth = GridWidth * CellSize;
    inline constexpr int WindowHeight = HudHeight + GridHeight * CellSize;

    inline constexpr int InitialSnakeLength = 4;
    inline constexpr int ScorePerFood = 10;

    inline constexpr float StartMoveInterval = 0.15f;
    inline constexpr float MinMoveInterval = 0.07f;
    inline constexpr float ScoreSpeedFactor = 0.0002f;
}

inline Position DirectionOffset(Direction direction)
{
    switch (direction)
    {
        case Direction::Up:    return {0, -1};
        case Direction::Down:  return {0, 1};
        case Direction::Left:  return {-1, 0};
        case Direction::Right: return {1, 0};
    }
    return {0, 0};
}

inline bool AreOpposite(Direction first, Direction second)
{
    Position a = DirectionOffset(first);
    Position b = DirectionOffset(second);
    return a.x == -b.x && a.y == -b.y;
}
