/**
 * @file Score.cpp
 * @brief Implementation of Score: run scoring plus high score file persistence.
 */

#include "Score.h"

#include <fstream>

#include "raylib.h"
#include "Types.h"

Score::Score()
    : current(0)
    , high(0)
{
    // GetApplicationDirectory() points at the executable, so the file is found
    // no matter where the game was started from (double-click, IDE, make run).
    highScorePath = std::string(GetApplicationDirectory()) + "highscore.txt";
    Load();
}

void Score::ResetRun()
{
    current = 0;
}

void Score::AddFood()
{
    current += Config::ScorePerFood;
}

bool Score::FinishRun()
{
    if (current > high)
    {
        high = current;
        Save();
        return true;
    }

    return false;
}

int Score::GetCurrent() const
{
    return current;
}

int Score::GetHigh() const
{
    return high;
}

void Score::Load()
{
    std::ifstream file(highScorePath);

    if (file.is_open())
    {
        file >> high;
    }

    // A missing file leaves `high` at 0; a negative or non-numeric value means
    // the file is corrupt, so reset it rather than displaying nonsense.
    if (high < 0)
    {
        high = 0;
    }
}

void Score::Save() const
{
    std::ofstream file(highScorePath);

    if (file.is_open())
    {
        file << high;
    }
}
