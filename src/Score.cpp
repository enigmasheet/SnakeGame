#include "Score.h"

#include <fstream>

#include "raylib.h"
#include "Types.h"

Score::Score()
    : current(0)
    , high(0)
{
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
