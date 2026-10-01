/**
 * @file logic_test.cpp
 * @brief Headless unit tests for Snake, Food, and Score.
 *
 * No window or audio device is required — only entity logic is exercised, so the
 * suite runs anywhere via `make test`. Each test prints one line on success;
 * a failed assert aborts the program with a non-zero exit code.
 */

#include <cassert>
#include <cstdio>

#include "Food.h"
#include "Score.h"
#include "Snake.h"

// assert() *is* the test framework here, so a build with NDEBUG would compile
// every check away and print "ALL LOGIC TESTS PASSED" without testing anything.
#ifdef NDEBUG
#error "logic_test.cpp must be built with asserts enabled; remove NDEBUG"
#endif

/**
 * @brief A default snake is 4 cells long, faces Right, and predicts a head
 *        exactly one cell ahead of the current one.
 */
static void TestInitialState()
{
    Snake snake;
    assert(snake.GetLength() == static_cast<std::size_t>(Config::InitialSnakeLength));
    assert(snake.GetDirection() == Direction::Right);

    const Position head = snake.GetBody().front();
    const Position predicted = snake.PredictHead();
    assert(predicted.x == head.x + 1 && predicted.y == head.y);
    std::printf("initial state ok\n");
}

/**
 * @brief Queuing a 180 degree reversal is ignored, while two valid turns
 *        (Up, then Down) are buffered and applied one step at a time.
 */
static void TestOppositeTurnRejected()
{
    Snake snake;
    snake.QueueDirection(Direction::Left);
    snake.Step(false);

    assert(snake.GetDirection() == Direction::Right);

    snake.QueueDirection(Direction::Up);
    snake.QueueDirection(Direction::Down);
    snake.Step(false);
    assert(snake.GetDirection() == Direction::Up);

    std::printf("180 degree rejection ok\n");
}

/**
 * @brief Only two turns can be buffered. A third request that would otherwise
 *        be legal is dropped, which is what keeps PredictHead() and Step()
 *        agreeing on which turn is applied next.
 */
static void TestQueueCap()
{
    Snake snake;

    snake.QueueDirection(Direction::Up);
    snake.QueueDirection(Direction::Left);
    snake.QueueDirection(Direction::Down); // legal on its own, but the buffer is full

    const std::deque<Direction>& pending = snake.GetPendingDirections();
    assert(pending.size() == 2);
    assert(pending.front() == Direction::Up);
    assert(pending.back() == Direction::Left);

    std::printf("queue cap ok\n");
}

/**
 * @brief PredictHead() reads the buffered turn before Step() consumes it:
 *        the peek must predict the cell Step() then actually lands on, without
 *        either call moving anything early.
 */
static void TestPredictHeadFollowsQueuedTurn()
{
    Snake snake;
    const Position head = snake.GetBody().front();

    snake.QueueDirection(Direction::Up);

    // Peek: the buffered turn is already reflected in the prediction.
    const Position predicted = snake.PredictHead();
    assert(predicted.x == head.x && predicted.y == head.y - 1);
    assert(snake.GetBody().front() == head);
    assert(snake.GetPendingDirections().size() == 1);

    // Pop: Step() consumes that same entry, so the head lands where predicted.
    snake.Step(false);
    assert(snake.GetBody().front() == predicted);
    assert(snake.GetDirection() == Direction::Up);
    assert(snake.GetPendingDirections().empty());

    std::printf("queued turn prediction ok\n");
}

/**
 * @brief Step(true) grows the snake by one cell; Step(false) keeps the length
 *        unchanged (the tail slides forward).
 */
static void TestGrowth()
{
    Snake snake;
    const std::size_t lengthBefore = snake.GetLength();

    snake.Step(true);
    assert(snake.GetLength() == lengthBefore + 1);

    snake.Step(false);
    assert(snake.GetLength() == lengthBefore + 1);

    snake.Step(false);
    assert(snake.GetLength() == lengthBefore + 1);
    std::printf("growth ok\n");
}

/**
 * @brief Enough steps to the right carry the head past the grid edge, at which
 *        point HitsWall() reports the collision.
 */
static void TestWallCollision()
{
    Snake snake;
    assert(!snake.HitsWall());

    // Wide enough to walk past the right edge from any configured start column:
    // the head begins at GridWidth / 3, so GridWidth + length + 1 steps always
    // overshoots, whatever Config::GridWidth is set to.
    const int steps = Config::GridWidth + Config::InitialSnakeLength + 1;

    for (int i = 0; i < steps; i++)
    {
        snake.Step(false);
    }

    assert(snake.HitsWall());
    std::printf("wall collision ok\n");
}

/**
 * @brief Driving back into its own body (Down, Left, Up while growing) is
 *        reported as a self collision.
 */
static void TestSelfCollision()
{
    Snake snake;

    snake.Step(true);
    snake.QueueDirection(Direction::Down);
    snake.Step(true);
    snake.QueueDirection(Direction::Left);
    snake.Step(true);
    snake.QueueDirection(Direction::Up);
    snake.Step(true);

    assert(snake.HitsItself());
    std::printf("self collision ok\n");
}

/**
 * @brief IsWithinGrid() accepts every cell of the board and rejects every cell
 *        one step outside it — the edges are exclusive on the right/bottom,
 *        which is what the wall check in Game relies on.
 */
static void TestGridBounds()
{
    assert(IsWithinGrid({0, 0}));
    assert(IsWithinGrid({Config::GridWidth - 1, Config::GridHeight - 1}));

    assert(!IsWithinGrid({-1, 0}));
    assert(!IsWithinGrid({0, -1}));
    assert(!IsWithinGrid({Config::GridWidth, 0}));
    assert(!IsWithinGrid({0, Config::GridHeight}));

    std::printf("grid bounds ok\n");
}

/**
 * @brief Across 500 respawns the food never lands on a snake cell and always
 *        stays within the grid bounds.
 */
static void TestFoodNeverSpawnsInsideSnake()
{
    Food food;
    Snake snake;

    for (int i = 0; i < 500; i++)
    {
        food.Spawn(snake.GetBody());

        bool inside = false;

        for (const Position& segment : snake.GetBody())
        {
            if (food.GetPosition() == segment)
            {
                inside = true;
            }
        }

        assert(!inside);
        assert(food.GetPosition().x >= 0 && food.GetPosition().x < Config::GridWidth);
        assert(food.GetPosition().y >= 0 && food.GetPosition().y < Config::GridHeight);
    }

    std::printf("food spawn ok\n");
}

/**
 * @brief The difficulty curve starts at Config::StartMoveInterval, decreases as
 *        the score rises, and is clamped at Config::MinMoveInterval.
 */
static void TestMoveInterval()
{
    // No score yet: exactly the configured starting pace.
    assert(ComputeMoveInterval(0) == Config::StartMoveInterval);

    // More points always means a shorter interval (a faster snake).
    assert(ComputeMoveInterval(200) < ComputeMoveInterval(100));

    // The floor is hit at 400 points: (0.15 - 0.07) / 0.0002 = 400.
    // A tiny tolerance absorbs float rounding at the exact boundary.
    assert(ComputeMoveInterval(400) <= Config::MinMoveInterval + 1e-3f);

    // The clamp is one-sided: never slower than the floor, never faster than it.
    assert(ComputeMoveInterval(1000000) >= Config::MinMoveInterval);
    assert(ComputeMoveInterval(1000000) == Config::MinMoveInterval);

    std::printf("move interval ok\n");
}

/**
 * @brief ResetRun() zeroes the counter and each AddFood() awards
 *        Config::ScorePerFood points.
 */
static void TestScore()
{
    Score score;
    score.ResetRun();
    assert(score.GetCurrent() == 0);

    score.AddFood();
    score.AddFood();
    assert(score.GetCurrent() == 2 * Config::ScorePerFood);

    std::printf("score ok\n");
}

/**
 * @brief Runs every test case in order.
 * @return 0 on success; a failed assert terminates the process before this line.
 */
int main()
{
    TestInitialState();
    TestOppositeTurnRejected();
    TestQueueCap();
    TestPredictHeadFollowsQueuedTurn();
    TestGrowth();
    TestWallCollision();
    TestSelfCollision();
    TestGridBounds();
    TestFoodNeverSpawnsInsideSnake();
    TestMoveInterval();
    TestScore();

    std::printf("ALL LOGIC TESTS PASSED\n");
    return 0;
}
