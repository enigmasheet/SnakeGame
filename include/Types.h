/**
 * @file Types.h
 * @brief Shared value types: grid coordinates, directions, game state, and tuning constants.
 *
 * Everything several translation units need to speak about the board lives here,
 * so entities can reference each other without including one another.
 */
#pragma once

/**
 * @brief A single cell coordinate on the play grid.
 *
 * Coordinates are cell indices, never pixels: `{0, 0}` is the top-left cell of
 * the play area. The HUD strip sits above the grid and is added only at draw time.
 */
struct Position
{
    int x = 0;
    int y = 0;

    /** @brief Exact coordinate equality, used for food pickup and self-collision. */
    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }

    /** @brief Negation of operator==. */
    bool operator!=(const Position& other) const
    {
        return !(*this == other);
    }
};

/** @brief One of the four legal travel directions on the grid. */
enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

/** @brief The four states of the game state machine. */
enum class GameState
{
    Menu,     /**< Title screen; Enter, Space, or R starts a run. */
    Playing,  /**< A run is live and the snake is moving. */
    Paused,   /**< Simulation and timer frozen; can resume, restart, or exit. */
    GameOver  /**< Run ended; final score shown until restart. */
};

/**
 * @brief Tuning constants for board geometry, scoring, and speed.
 *
 * Named constants instead of magic numbers so layout and difficulty can be
 * adjusted in one place. Window size is derived from the grid to keep them in sync.
 */
namespace Config
{
    // --- Board geometry (grid cells are CellSize x CellSize pixels) ---
    inline constexpr int CellSize = 25;
    inline constexpr int GridWidth = 30;
    inline constexpr int GridHeight = 20;
    inline constexpr int HudHeight = 60;
    inline constexpr int WindowWidth = GridWidth * CellSize;
    inline constexpr int WindowHeight = HudHeight + GridHeight * CellSize;

    // --- Run rules ---
    inline constexpr int InitialSnakeLength = 4;
    inline constexpr int ScorePerFood = 10;

    // --- Speed: interval shrinks with score, never below the minimum ---
    inline constexpr float StartMoveInterval = 0.15f;
    inline constexpr float MinMoveInterval = 0.07f;
    inline constexpr float ScoreSpeedFactor = 0.0002f;
}

/**
 * @brief Grid delta for travelling one cell in @p direction.
 * @param direction Travel direction.
 * @return Offset `{dx, dy}` in cells. The trailing `return {0, 0}` only silences
 *         a "control reaches end of non-void function" warning; every enumerator
 *         returns above it.
 */
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

/**
 * @brief Whether two directions form a 180 degree reversal.
 * @param first  Direction to test.
 * @param second Direction to compare against.
 * @return true when the two offsets point exactly away from each other.
 *         Equal directions return false: they are rejected separately by
 *         Snake::QueueDirection() as duplicates.
 */
inline bool AreOpposite(Direction first, Direction second)
{
    Position a = DirectionOffset(first);
    Position b = DirectionOffset(second);
    return a.x == -b.x && a.y == -b.y;
}

/**
 * @brief Whether a cell lies inside the play grid.
 *
 * The single source of truth for bounds checks: the game applies it to the
 * predicted head *before* stepping, while Snake::HitsWall applies it to the
 * current head. Keeping one implementation means a resize of the board only
 * needs a change here (through Config).
 *
 * @param cell Cell to test.
 * @return true when 0 <= x < GridWidth and 0 <= y < GridHeight.
 */
inline bool IsWithinGrid(const Position& cell)
{
    return cell.x >= 0 && cell.x < Config::GridWidth
        && cell.y >= 0 && cell.y < Config::GridHeight;
}

/**
 * @brief Seconds between moves for a given score — the difficulty curve.
 *
 * Written as a free function rather than a method so it can be unit-tested
 * without a Game (and therefore without a window).
 *
 * @param score Points scored in the current run.
 * @return Config::StartMoveInterval shrunk by Config::ScoreSpeedFactor per
 *         point, clamped at Config::MinMoveInterval. The floor is reached at
 *         400 points (40 foods): (0.15 - 0.07) / 0.0002 = 400.
 */
inline float ComputeMoveInterval(int score)
{
    const float interval = Config::StartMoveInterval - score * Config::ScoreSpeedFactor;

    return interval < Config::MinMoveInterval ? Config::MinMoveInterval : interval;
}
