#include "Audio.h"

#include <cmath>

namespace
{
    constexpr int SampleRate = 44100;
    constexpr double Pi = 3.14159265358979323846;
}

Audio::Audio()
    : ready(false)
    , eat{}
    , gameOver{}
    , click{}
{
    InitAudioDevice();

    if (!IsAudioDeviceReady())
    {
        return;
    }

    eat = LoadSoundFromWave(MakeTone(520.0f, 880.0f, 0.12f, 0.45f));
    gameOver = LoadSoundFromWave(MakeTone(420.0f, 110.0f, 0.5f, 0.5f));
    click = LoadSoundFromWave(MakeTone(700.0f, 700.0f, 0.05f, 0.3f));

    ready = true;
}

Audio::~Audio()
{
    if (!ready)
    {
        CloseAudioDevice();
        return;
    }

    UnloadSound(eat);
    UnloadSound(gameOver);
    UnloadSound(click);
    CloseAudioDevice();
}

void Audio::PlayEat() const
{
    Play(eat);
}

void Audio::PlayGameOver() const
{
    Play(gameOver);
}

void Audio::PlayClick() const
{
    Play(click);
}

Wave Audio::MakeTone(float startFrequency, float endFrequency, float duration, float volume)
{
    const int frameCount = static_cast<int>(SampleRate * duration);
    auto* samples = static_cast<short*>(MemAlloc(frameCount * sizeof(short)));

    double phase = 0.0;

    for (int i = 0; i < frameCount; i++)
    {
        const float t = static_cast<float>(i) / static_cast<float>(frameCount);
        const float frequency = startFrequency + (endFrequency - startFrequency) * t;

        phase += 2.0 * Pi * frequency / SampleRate;

        const float envelope = volume * (1.0f - t);
        const float value = std::sin(static_cast<float>(phase)) * envelope;

        samples[i] = static_cast<short>(value * 32767.0f);
    }

    Wave wave{};
    wave.frameCount = static_cast<unsigned int>(frameCount);
    wave.sampleRate = SampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples;

    return wave;
}

void Audio::Play(const Sound& sound) const
{
    if (ready)
    {
        PlaySound(sound);
    }
}
