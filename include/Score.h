/**
 * @file Score.h
 * @brief Run score plus the persistent high score loaded/saved from highscore.txt.
 */
#pragma once

#include <string>

/**
 * @brief Tracks the score of the current run and the all-time best.
 *
 * The high score lives in `highscore.txt` next to the executable so it survives
 * between launches. Every file access is best-effort: a missing or unwritable
 * file degrades to 0 instead of aborting the game.
 */
class Score
{
public:
    /** @brief Resolves the high score path (next to the executable) and loads the stored value. */
    Score();

    /** @brief Clears the active run's score. The stored high score is untouched. */
    void ResetRun();

    /** @brief Awards Config::ScorePerFood points for a piece of food. */
    void AddFood();

    /**
     * @brief Ends the run, persisting a new high score if one was beaten.
     *
     * Call once when the run finishes, before the game-over screen is drawn.
     *
     * @return true when the current score exceeded the stored high score and
     *         was written to disk (drives the "NEW BEST SCORE" banner);
     *         false when the old record still stands.
     */
    bool FinishRun();

    /** @return Points scored in the active run. */
    int GetCurrent() const;

    /** @return Best score (from disk, or 0 when missing/corrupt). */
    int GetHigh() const;

private:
    /**
     * @brief Reads the stored high score.
     * Missing files leave the value at 0; a negative or non-numeric value is
     * clamped back to 0 so a corrupted file cannot poison the display.
     */
    void Load();

    /** @brief Writes the high score back out; silently skipped if the file cannot be opened. */
    void Save() const;

    int current;              /**< Points in the active run. */
    int high;                 /**< Best score, possibly just loaded from disk. */
    std::string highScorePath; /**< Full path to highscore.txt. */
};
