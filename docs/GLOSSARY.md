# Glossary

C++ and game-programming terms, each explained the way this project uses them.
Cross-references are to the files where the term shows up.

---

### Accumulator (fixed timestep)
Keeping a running sum of elapsed time and spending it in fixed increments, so the
simulation runs at a steady rate regardless of how often the screen refreshes.
`Game::Update()` in `src/Game.cpp` adds `GetFrameTime()` to `moveTimer` and
drains it with a `while` loop.

### Anonymous namespace
A namespace with no name; symbols declared inside have *internal linkage* — they
exist only in that one `.cpp` file and cannot be referenced (or clash with
something) elsewhere. Used for the shared color palette and small helpers like
`DrawCenteredText()` in `src/Game.cpp`.

### Argument vs parameter
An **argument** is what you pass at a call site; a **parameter** is the name that
receives it in the declaration. `Game::ResetBoard(GameState next)` — `next` is
the parameter.

### Assert
A runtime check that aborts the program when a condition is false. Used in
`tests/logic_test.cpp` (`assert(snake.GetBody().size() == 5);`) instead of an
exception, because a failing test must be loud and non-recoverable.

### Body / definition
The *declaration* says a function exists (`void Draw() const;` in a header); the
*definition* is the implementation (`Game.cpp`). Headers declare, `.cpp` files
define — that is why one header can be included by many translation units without
causing duplicate-symbol link errors.

### Build
Everything from source to executable: **compile** each `.cpp` into an object
file, then **link** the objects plus libraries into `snake.exe`. See
[BUILD.md](BUILD.md).

### Class
A user-defined type bundling data and the functions that operate on it, with
`private` members hidden from outside. `Game`, `Snake`, `Food`, `Score`, and
`Audio` are the five classes here.

### Composition
Building a type out of other types as members, rather than inheriting from them.
`Game` *has a* `Snake`, `Food`, `Score`, and `Audio`. This project deliberately
uses composition instead of inheritance — see the note in
[TEACHING.md](TEACHING.md#3-class-structure).

### Const
Promises "read only". `int GetLength() const` will not modify the object;
`const std::vector<Position>&` prevents the caller's vector from being changed.
Applied to nearly every query in the headers.

### Container
A standard-library type holding a collection of elements: `std::vector` (the
snake body), `std::deque` (the pending-turn queue).

### Cell vs pixel
A **cell** is a grid coordinate like `(4, 7)`; a **pixel** is a screen
coordinate. Cells are multiplied by `Config::CellSize` (25) only inside `Draw()`.
Keeping gameplay in cells is what makes collision detection integer equality.

### Current working directory (CWD)
The folder the process was started from. It is *not* stable — `make run`,
double-clicking the exe, and pressing F5 in VS Code all differ — which is why the
high score uses `GetApplicationDirectory()` instead of a relative path.

### deque
Double-ended queue: efficient insertion and removal at **both** ends. Used for
`pendingDirections` because turns are appended at the back and consumed at the
front (`src/Snake.cpp`).

### Dependency file (`.d`)
A file written by `-MMD` listing which headers an object was built from. `make`
reads it to recompile exactly what a header change invalidated. Generated into
`build/`.

### Doxygen
The comment convention `/** @brief ... @param ... @return ... */` that
documentation generators parse. Headers carry the API docs; `.cpp` files carry
short `//` notes on the tricky parts.

### Encapsulation
Keeping state `private` and exposing behaviour through methods. `Snake`'s body
vector is private; callers use `GetBody()`, `QueueDirection()`, `Step()` — so
invariants (like "the head is always at index 0") cannot be broken from outside.

### enum class (scoped enumeration)
A type-safe named set of values that does **not** leak into the enclosing
namespace and does not implicitly convert to `int`.
`enum class Direction { Up, Down, Left, Right }` and
`enum class GameState { Menu, Playing, Paused, GameOver }` in `include/Types.h`.

### Frame
One pass through the loop: input → update → draw. The window runs at about 60
frames per second (`SetTargetFPS(60)` in `src/main.cpp`).

### Getter
A method that returns a value without changing anything, almost always `const`:
`Snake::GetLength()`, `Score::GetHigh()`, `Food::GetPosition()`.

### Header file (`.h`)
A declaration file included where the type is needed. Splitting declarations
(`include/`) from implementations (`src/`) limits recompilation and states the
public interface explicitly.

### `#pragma once`
Include guard equivalent placed at the top of every header here. It stops the
file from being processed twice in one translation unit, which would otherwise
produce redefinition errors.

### Include path (`-Iinclude`)
The directory `g++` searches for quoted includes. The Makefile passes `-Iinclude`
so `#include "Types.h"` resolves without writing `../include/Types.h`.

### Inline variable (C++17)
`inline constexpr int CellSize = 25;` — declared in a header, defined once, and
legal to include in many files without duplicate-definition errors. Used for all
constants in `include/Types.h`.

### Instance
A particular object of a class. There is exactly one `Game` instance, created on
the stack in `main()`.

### Linker / linking
The stage that resolves symbol names across object files and libraries and
produces the final `.exe`. Errors like `undefined reference to 'glfwInit'` are
*link* errors, not compile errors.

### Literal suffix `f`
`0.15f` is a `float` literal; without the `f` it would be a `double`, forcing a
conversion. Every timing constant here carries the suffix.

### Namespace
A named scope for grouping symbols so they cannot collide. The game uses
`Config` for tuning constants and the unnamed (anonymous) namespace for
file-local helpers.

### Object file (`.o`)
The binary produced by compiling one `.cpp`. Multiple objects are linked into
one executable; they live in `build/` and are never committed.

### Object vs reference vs pointer
All three can name the same object. **Object** (`Snake snake;`) owns it.
**Reference** (`const Position&`) is an alias that must be bound and never
rebound — used everywhere for parameters. **Pointer** (`Position*`) can be null
and reassigned — deliberately avoided in this codebase because ownership is
expressed by value instead.

### OOP (object-oriented programming)
Organising a program around objects that pair state with behaviour. This project
uses encapsulation, composition, and message passing (method calls). It does
**not** use inheritance or runtime polymorphism — see the trade-off discussion in
[TEACHING.md](TEACHING.md#3-class-structure).

### Overload
Several functions with the same name but different parameter lists. `Game` has
the public `StartRun()` plus the private `ResetBoard(GameState)`, which keep
similar behaviour separate without overloading confusion.

### Private / public
Access specifiers. `public` is the contract; `private` is implementation
detail. Every member of `Snake`, `Food`, `Score`, and `Audio` is private with a
public method to reach it.

### RAII (Resource Acquisition Is Initialization)
The object's constructor acquires a resource and its destructor releases it, so
lifetime and scope are the same thing. `Audio` opens the audio device and
synthesizes sounds in its constructor and unloads them in its destructor — no
`close()` call is needed, and an early `return` cannot leak.

### Random engine and distribution
`std::mt19937` produces raw pseudo-random bits;
`std::uniform_int_distribution(0, ...)` turns them into unbiased integers in a
range. Using both is the modern replacement for `rand() % n` (which is biased).
See `Food::Spawn()`.

### Reference (`&`)
An alias for an existing object: `const std::vector<Position>& GetBody() const`
hands out the body without copying it and without allowing modification. The
caller's `snake` object is untouched.

### Rejection sampling
Draw a candidate, reject it if illegal, repeat. Used by `Food::Spawn()` so food
never lands on the snake. Cheap because the board is large relative to the snake.

### State machine
A value that selects which block of behaviour is active. `GameState` plus the
`switch` in `Game::HandleInput()` decide what each key means.

### static — three different meanings
1. **`static` member function/variable** (`Audio::sInitialized`): belongs to the
   class, not to any instance.
2. **`static` in a function** (`static std::mt19937 rng;`): initialised once and
   persists across calls.
3. **Static linking** (`-static-libgcc`, `libraylib.a`): code copied into the
   executable instead of being loaded from a DLL at runtime.

### Struct vs class
Identical except for the default access level: `struct` defaults to `public`,
`class` to `private`. `Position` is a `struct` (plain data); the five types with
invariants are `class`es.

### std::
The C++ standard library namespace: `std::vector`, `std::string`,
`std::find`, `std::mt19937`, `std::ifstream`.

### Symbol
A named entity the compiler emits (a function or variable name). The linker
matches references to definitions; `undefined reference` means no definition was
supplied.

### Translation unit
A `.cpp` plus everything it (transitively) includes — what the compiler turns
into one object file. `Game.cpp` including `Game.h` which includes `raylib.h` is
one translation unit.

### Undefined behaviour (UB)
Behaviour the standard leaves arbitrary — out-of-bounds indexing, dangling
references, use after free. The grid checks (`IsWithinGrid()`) exist precisely to
keep every index inside `[0, 600)`.

### Unit test
A test that exercises one unit of logic in isolation. `tests/logic_test.cpp`
proves movement, collisions, spawning, timing, and scoring with no window,
audio, or timing dependency.

### vector
Contiguous, resizable array with O(1) indexing. The snake body:
`std::vector<Position>` in `include/Snake.h`. Growth is `push_front` at the head
and `pop_back` at the tail each step (unless food was eaten).

### Virtual function / vtable
Mechanism behind runtime polymorphism (calling a derived implementation through
a base pointer). **Not used in this project** — with one implementation per type,
an `enum` state machine is simpler and faster than a virtual call per key press.

### Warning flags
`-Wall -Wextra` enable diagnostics such as sign-compare and unused-parameter.
This project is expected to build with zero warnings; warnings are treated as
defects, not noise.

---

## Quick reference: where each concept lives

| Concept | File |
| ------- | ---- |
| constants, `enum class`, grid math | `include/Types.h` |
| loop, scoping, teardown order | `src/main.cpp` |
| `vector` body, `deque` queue, `const` API | `include/Snake.h`, `src/Snake.cpp` |
| `mt19937`, rejection sampling | `include/Food.h`, `src/Food.cpp` |
| file streams, error handling | `include/Score.h`, `src/Score.cpp` |
| RAII, deleted copies, `static` members | `include/Audio.h`, `src/Audio.cpp` |
| state machine, accumulator, collisions | `include/Game.h`, `src/Game.cpp` |
| `assert`, testable design | `tests/logic_test.cpp` |

Also see [TEACHING.md](TEACHING.md) (algorithms, exercises, viva Q&A) and
[BUILD.md](BUILD.md) (toolchain and troubleshooting).
