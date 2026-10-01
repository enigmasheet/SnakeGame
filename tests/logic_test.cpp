#include <cassert>
#include <cstdio>

#include "Food.h"
#include "Score.h"
#include "Snake.h"

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

static void TestFoodNeverSpawnsInsideSnake()
{
    Food food;
    Snake snake;

    for (int i = 0; i < 500; i++)
    {
        food.Spawn(snake.GetBody());

        const Position& head = snake.GetBody().front();
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
        (void)head;
    }

    std::printf("food spawn ok\n");
}

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

int main()
{
    TestInitialState();
    TestOppositeTurnRejected();
    TestGrowth();
    TestWallCollision();
    TestSelfCollision();
    TestFoodNeverSpawnsInsideSnake();
    TestScore();

    std::printf("ALL LOGIC TESTS PASSED\n");
    return 0;
}
