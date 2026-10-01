/**
 * @file Audio.h
 * @brief Audio device ownership and the three procedurally synthesized sound effects.
 */
#pragma once

#include "raylib.h"

/**
 * @brief Owns the audio device and plays the eat, game-over, and click sounds.
 *
 * The sounds are synthesized at construction as decaying sine sweeps, so the
 * project ships with no binary assets. If no audio device is available every
 * play call becomes a no-op instead of crashing.
 *
 * Non-copyable: the Sound objects belong to the device this instance closes in
 * its destructor, so a second copy would unload sounds still in use.
 */
class Audio
{
public:
    /** @brief Opens the audio device and synthesizes the three sounds (no-ops if the device never becomes ready). */
    Audio();

    /** @brief Unloads the sounds, then closes the device — the reverse of construction order. */
    ~Audio();

    /** @brief Single-owner resource; copying would double-free the underlying sounds. */
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    /** @brief Short rising sweep, played when food is eaten. */
    void PlayEat() const;

    /** @brief Long falling sweep, played when the snake dies. */
    void PlayGameOver() const;

    /** @brief Short blip, played on menu and pause clicks. */
    void PlayClick() const;

private:
    /**
     * @brief Synthesizes a mono 16-bit tone with a linear frequency sweep and a
     *        decaying envelope.
     *
     * The phase is accumulated sample by sample (rather than recomputing from
     * absolute time) because the frequency changes continuously; a single
     * post-increment sine would otherwise keep one frequency and sweep nothing.
     *
     * @param startFrequency Frequency (Hz) at the first sample.
     * @param endFrequency   Frequency (Hz) at the last sample.
     * @param duration       Length in seconds.
     * @param volume         Peak amplitude in [0, 1].
     * @return Wave whose samples come from MemAlloc(); ownership passes to
     *         raylib and the buffer is released later by UnloadSound().
     */
    static Wave MakeTone(float startFrequency, float endFrequency, float duration, float volume);

    /** @brief Plays @p sound, or does nothing if the device is not ready. */
    void Play(const Sound& sound) const;

    bool ready;     /**< True only when the device started and the sounds loaded. */
    Sound eat;      /**< Rising sweep. */
    Sound gameOver; /**< Falling sweep. */
    Sound click;    /**< Short blip. */
};
