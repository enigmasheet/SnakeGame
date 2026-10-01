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
    // Every way the file can be unusable means the same thing — there is no
    // record yet — so one test covers a missing file (the stream never opens),
    // a non-numeric or empty file (extraction fails), and a negative value.
    std::ifstream file(highScorePath);
    int stored = 0;

    if (!(file >> stored) || stored < 0)
    {
        stored = 0;
    }

    high = stored;
}

void Score::Save() const
{
    std::ofstream file(highScorePath);

    if (file.is_open())
    {
        file << high;
    }
}
