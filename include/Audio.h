#pragma once

#include "raylib.h"

class Audio
{
public:
    Audio();
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    void PlayEat() const;
    void PlayGameOver() const;
    void PlayClick() const;

private:
    static Wave MakeTone(float startFrequency, float endFrequency, float duration, float volume);
    void Play(const Sound& sound) const;

    bool ready;
    Sound eat;
    Sound gameOver;
    Sound click;
};
