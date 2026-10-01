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

/**
 * @brief A default snake is 4 cells long, faces Right, and predicts a head
 *        exactly one cell ahead of the current one.
 */
static void TestInitialState()
{
    Snake snake;
    assert(snake.GetLength() == 4);
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

    for (int i = 0; i < 60; i++)
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
    assert(score.GetCurrent() == 20);

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
    TestGrowth();
    TestWallCollision();
    TestSelfCollision();
    TestFoodNeverSpawnsInsideSnake();
    TestMoveInterval();
    TestScore();

    std::printf("ALL LOGIC TESTS PASSED\n");
    return 0;
}
