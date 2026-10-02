/**
 * @file Audio.cpp
 * @brief Implementation of Audio: procedural tone synthesis and playback.
 */

#include "Audio.h"

#include <cmath>

#include "raylib.h"

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
    // Open the device first. When no audio hardware is available the constructor
    // returns here and every Play() becomes a no-op, so the game still runs.
    InitAudioDevice();

    if (!IsAudioDeviceReady())
    {
        return;
    }

    // Three synthesized cues: eat sweeps up (reward), game over sweeps down
    // (defeat), and the click is a short neutral blip for menu feedback.
    eat = LoadSoundFromWave(MakeTone(520.0f, 880.0f, 0.12f, 0.45f));
    gameOver = LoadSoundFromWave(MakeTone(420.0f, 110.0f, 0.5f, 0.5f));
    click = LoadSoundFromWave(MakeTone(700.0f, 700.0f, 0.05f, 0.3f));

    ready = true;
}

Audio::~Audio()
{
    // Mirror construction in reverse: release the wave buffers while the device
    // that owns them is still open, then shut the device down.
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
        // Linear sweep from startFrequency to endFrequency over the sample.
        const float t = static_cast<float>(i) / static_cast<float>(frameCount);
        const float frequency = startFrequency + (endFrequency - startFrequency) * t;

        // Accumulate phase per sample: the frequency changes continuously, so a
        // sine of absolute time would collapse to one fixed pitch.
        phase += 2.0 * Pi * frequency / SampleRate;

        // Linear decay from `volume` to silence, so the note fades out rather
        // than ending with an audible click.
        const float envelope = volume * (1.0f - t);
        const float value = std::sin(static_cast<float>(phase)) * envelope;

        samples[i] = static_cast<short>(value * 32767.0f);
    }

    Wave wave{};

    // 16-bit signed PCM at 44100 Hz, mono — the format LoadSoundFromWave expects
    // for a plain buffer. `samples` (from MemAlloc) is now owned by this Wave and
    // freed by UnloadSound().
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
