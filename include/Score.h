#pragma once

#include <string>

class Score
{
public:
    Score();

    void ResetRun();
    void AddFood();
    bool FinishRun();

    int GetCurrent() const;
    int GetHigh() const;

private:
    void Load();
    void Save() const;

    int current;
    int high;
    std::string highScorePath;
};
