/**
 * @file Snake.cpp
 * @brief Implementation of Snake: reset placement, buffered turns, movement, and drawing.
 */

#include "Snake.h"

#include "raylib.h"

namespace
{
    const Color HeadColor = {130, 235, 140, 255};
    const Color BodyColorStart = {66, 196, 90, 255};
    const Color BodyColorEnd = {38, 120, 62, 255};
    const Color EyeColor = {16, 24, 18, 255};
}

Snake::Snake()
{
    Reset();
}

void Snake::Reset()
{
    body.clear();
    pendingDirections.clear();

    // Start on the middle row a third of the way in, with the tail extending to
    // the left so the snake travels right immediately after the first key press.
    const int startY = Config::GridHeight / 2;
    const int startX = Config::GridWidth / 3;

    for (int i = 0; i < Config::InitialSnakeLength; i++)
    {
        body.push_back({startX - i, startY});
    }

    direction = Direction::Right;
}

void Snake::QueueDirection(Direction newDirection)
{
    // Compare against the last *queued* direction, not the applied one: two
    // presses can land between two steps, and "Up then Down" must not be
    // treated as valid even though the snake is still travelling Right.
    const Direction lastDirection = pendingDirections.empty() ? direction : pendingDirections.back();

    if (AreOpposite(lastDirection, newDirection) || lastDirection == newDirection)
    {
        return;
    }

    // Cap the buffer at two pending turns: enough to keep up with fast key
    // presses, small enough that prediction and application stay identical.
    if (pendingDirections.size() >= 2)
    {
        return;
    }

    pendingDirections.push_back(newDirection);
}

Position Snake::PredictHead() const
{
    // Peek at the queued turn without consuming it, so the caller can test the
    // result before anything moves. Step() pops this same entry, which keeps the
    // predicted cell and the cell the head actually occupies identical.
    const Direction nextDirection = pendingDirections.empty() ? direction : pendingDirections.front();
    const Position offset = DirectionOffset(nextDirection);

    return {body.front().x + offset.x, body.front().y + offset.y};
}

void Snake::Step(bool grow)
{
    // Consume the buffered turn (if any) before computing the offset, so the
    // new head lands in the requested direction.
    if (!pendingDirections.empty())
    {
        direction = pendingDirections.front();
        pendingDirections.pop_front();
    }

    const Position offset = DirectionOffset(direction);
    body.insert(body.begin(), {body.front().x + offset.x, body.front().y + offset.y});

    if (!grow)
    {
        body.pop_back();
    }
}

bool Snake::HitsWall() const
{
    const Position& head = body.front();

    return head.x < 0 || head.x >= Config::GridWidth || head.y < 0 || head.y >= Config::GridHeight;
}

bool Snake::HitsItself() const
{
    const Position& head = body.front();

    // Start at 1: segment 0 *is* the head, so comparing it would always match.
    for (std::size_t i = 1; i < body.size(); i++)
    {
        if (body[i] == head)
        {
            return true;
        }
    }

    return false;
}

void Snake::Draw() const
{
    const float inset = 2.0f;
    const float size = static_cast<float>(Config::CellSize) - inset * 2.0f;

    // Draw tail-first so the head (drawn last) is never covered by later
    // segments. The gradient parameter t runs 0 (tail) to 1 (neck).
    for (std::size_t i = body.size(); i-- > 0;)
    {
        const float t = body.size() > 1 ? static_cast<float>(i) / static_cast<float>(body.size() - 1) : 0.0f;
        const Color color = ColorLerp(BodyColorStart, BodyColorEnd, t);

        const float x = static_cast<float>(body[i].x * Config::CellSize) + inset;
        const float y = static_cast<float>(Config::HudHeight + body[i].y * Config::CellSize) + inset;

        DrawRectangleRounded({x, y, size, size}, 0.4f, 6, color);
    }

    const float headX = static_cast<float>(body.front().x * Config::CellSize);
    const float headY = static_cast<float>(Config::HudHeight + body.front().y * Config::CellSize);

    DrawRectangleRounded({headX + inset, headY + inset, size, size}, 0.4f, 6, HeadColor);

    const Position offset = DirectionOffset(direction);
    const float eyeSize = 4.0f;
    const float centerX = headX + Config::CellSize / 2.0f;
    const float centerY = headY + Config::CellSize / 2.0f;
    const float forward = 5.0f;
    const float sideways = 5.0f;

    // The eyes are pushed forward along the travel direction and sideways along
    // the perpendicular vector (-offset.y, offset.x), then mirrored with `side`.
    const Position sideOffset{-offset.y, offset.x};

    for (int side = -1; side <= 1; side += 2)
    {
        const float eyeX = centerX + offset.x * forward + sideOffset.x * sideways * side - eyeSize / 2.0f;
        const float eyeY = centerY + offset.y * forward + sideOffset.y * sideways * side - eyeSize / 2.0f;

        DrawRectangleRec({eyeX, eyeY, eyeSize, eyeSize}, EyeColor);
    }
}

const std::vector<Position>& Snake::GetBody() const
{
    return body;
}

Direction Snake::GetDirection() const
{
    return direction;
}

std::size_t Snake::GetLength() const
{
    return body.size();
}
