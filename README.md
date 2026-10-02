# Snake

A classic Snake game clone written in C++17 using [Raylib](https://www.raylib.com/) for rendering, input, and audio.

## Features

- Grid-based snake movement with WASD and arrow keys
- Food spawning that never overlaps the snake
- Growth, scoring, and wall / self collision detection
- Game states: main menu, playing, paused, game over
- Speed increases as your score grows (0.15s down to 0.07s per move)
- Persistent high score saved to `bin/highscore.txt`
- Sound effects generated procedurally in code (no asset files needed)
- Reversal (180 degree) input is rejected, with a 2-deep input queue for responsive turning
- F1 teaching overlay showing head/tail cells, the queued turns, and the move timer

## Controls

| Key | Action |
| --- | --- |
| Arrow keys / WASD | Steer the snake |
| Enter / Space | Start game / restart after game over |
| P | Pause / resume |
| Esc | Pause (while playing) / back to menu |
| R | Restart |
| F1 | Toggle the debug/teaching overlay |
| Window close / Alt+F4 | Quit (Esc deliberately never does — see `SetExitKey` in `src/main.cpp`) |

## Building

Requirements: MSYS2 with the UCRT64 toolchain and Raylib.

```
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make
```

Then, from an MSYS2 UCRT64 shell in the project folder:

```
make help     # list the available targets
make          # build bin/snake.exe (also copies glfw3.dll and libwinpthread-1.dll into bin/)
make run      # build and play
make test     # build and run the headless logic tests
make clean    # remove bin/ and build/
```

In VS Code, `Ctrl+Shift+B` runs the default build task, and the `run`, `test`, `clean` tasks are available under Terminal > Run Task. Pressing F5 debugs the game with GDB.

All build output goes to two generated folders: object files and dependency files in `build/`, and the executable, runtime DLLs, test binary, and `highscore.txt` in `bin/`. Nothing in either folder is tracked by git, so `bin/snake.exe` can also be double-clicked: `make` copies the required DLLs next to it.

## Learning and teaching

The project is written to be read as well as run. Three guides live in `docs/`:

- **[docs/TEACHING.md](docs/TEACHING.md)** — guided reading order, the core algorithms explained with diagrams, a concept map from each file to the C++ topics it demonstrates, graded exercises, and viva questions with answers.
- **[docs/BUILD.md](docs/BUILD.md)** — how the toolchain and the `Makefile` work, and a troubleshooting table for the errors beginners hit first.
- **[docs/GLOSSARY.md](docs/GLOSSARY.md)** — the C++ terms used in this codebase, each defined with an example drawn from the project.

Press **F1** in game to show the debug overlay: it prints the head and tail cells, the queued turns, and the live move timer, which makes the algorithms below observable at runtime.

## Project structure

```
SnakeGame/
├── include/                # public headers
│   ├── Game.h              # state machine: menu / playing / paused / game over
│   ├── Snake.h             # body vector, movement, growth, drawing, collision checks
│   ├── Food.h              # position, random spawn (never inside the snake), drawing
│   ├── Score.h             # current score, high score load/save
│   ├── Audio.h             # audio device and procedurally generated sound effects
│   └── Types.h             # Position struct, Direction/GameState enums, grid config
├── src/                    # implementations
│   ├── main.cpp            # window setup and the game loop (input -> update -> draw)
│   ├── Game.cpp            # state machine, fixed-step timing, collision resolution
│   ├── GameInput.cpp       # keyboard dispatch: one branch per state, F1 overlay toggle
│   ├── GameRender.cpp      # theme palette, baked board, HUD, and every overlay
│   ├── Snake.cpp / Food.cpp / Score.cpp / Audio.cpp
├── tests/
│   └── logic_test.cpp      # headless tests for movement, collisions, spawn, timing, scoring
├── docs/
│   ├── TEACHING.md         # guided tour, diagrams, exercises, viva Q&A
│   ├── BUILD.md            # toolchain + Makefile explained, troubleshooting
│   └── GLOSSARY.md         # C++ terms as used in this codebase
├── .gitattributes          # force LF line endings (keeps the Makefile valid)
├── .gitignore              # ignores build/, bin/, and OS/editor noise
├── .vscode/                # build, run, test, and debug configuration
├── Makefile
├── README.md
├── build/                  # generated: object and dependency files
└── bin/                    # generated: snake.exe, DLLs, logic_test.exe, highscore.txt
```

## Coding conventions

- **Includes, in this order, blank line between groups:** own header, C++ standard library, third-party (`raylib.h`), then other project headers (`Types.h`). A translation unit includes what it uses — do not rely on a header's transitive includes.
- **File headers:** every `.cpp`/`.h` opens with a Doxygen `/** @file … @brief … */` block that says what the file is for; public declarations in `include/` carry `@brief`, `@param`, `@return`.
- **Style:** Allman braces, 4-space indent, no tabs, LF line endings (enforced by `.gitattributes`), no trailing whitespace.
- **File-local constants and helpers** go in an anonymous namespace (internal linkage), never at global scope — see the color palettes in `GameRender.cpp` and `Snake.cpp`.
- **Comments explain *why*, not *what*.** Each non-obvious decision states the reason it exists.
- **No `using namespace`, no raw `new`/`delete`.** Entities are composed by value; the standard library covers ownership.
- **One concern per `.cpp`:** `Game.cpp` decides, `GameInput.cpp` reads the keyboard, `GameRender.cpp` draws.

## Design notes

- **Grid, not pixels.** The snake and food live on a 30 x 20 grid of 25 px cells; pixel coordinates are only computed at draw time (`x * CellSize`). Collision detection becomes simple integer equality.
- **Snake body** is a `std::vector<Position>`. Each move inserts a new head at the front and removes the tail, unless food was eaten, which grows it by one.
- **Movement timing** is decoupled from rendering: the game draws at 60 FPS but only steps the snake when the accumulated timer exceeds the current move interval, which shrinks as the score rises.
- **Input queue.** Direction changes are queued (max 2) and validated against the last queued direction, so quick double-turns work while 180 degree reversals are rejected.
- **Collision order.** The next head position is predicted first (wall check), then the snake steps, then the food and self-collision checks run against the updated body so moving into the cell the tail just vacated is legal.
- **Sound effects** are synthesized as decaying sine waves at startup and loaded with `LoadSoundFromWave`, so no binary assets are required.

## Testing

`make test` runs headless assertions covering: initial state, reversal rejection, the 2-deep turn buffer (including the cap and `PredictHead` predicting a queued turn), growth, wall collision, self collision, grid bounds, food spawn exclusion (500 iterations), the difficulty curve (start value, monotonicity, and the 400-point floor), and score accounting.

The tests link only the entity logic — no window or audio device — which is why they can run in seconds and prove the rules are correct independently of the renderer.
