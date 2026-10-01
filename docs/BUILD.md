# Building the project

This guide explains what happens between `make` and a running game, and how to
fix the errors that stop it working. If you only want commands, use
`make help`.

---

## 1. Prerequisites

Everything is installed through [MSYS2](https://www.msys2.org/) using the
**UCRT64** environment.

```
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make
```

| Package | Provides |
| ------- | -------- |
| `mingw-w64-ucrt-x86_64-gcc` | `g++` 16.2.0, the C++17 compiler |
| `mingw-w64-ucrt-x86_64-raylib` | `raylib.h`, static `libraylib.a`, `glfw3.dll` |
| `make` | GNU Make (build driver) |

Optional, for the F5 debugger: `pacman -S mingw-w64-ucrt-x86_64-gdb`.

**Open the "MSYS2 UCRT64" shell** (not plain "MSYS2"). The UCRT64 shell puts
`/ucrt64/bin` on `PATH`, which is what makes `g++` and `raylib.h` findable. A
plain MSYS shell will fail with `g++: command not found`.

---

## 2. The targets

Run `make help` for this list at any time.

| Command | What it does | Output |
| ------- | ------------ | ------ |
| `make` / `make all` | Compile and link the game | `bin/snake.exe` + 2 DLLs |
| `make run` | Build, then start the game | window opens |
| `make test` | Build and run the headless logic tests | `bin/logic_test.exe` |
| `make clean` | Delete all generated output | `build/`, `bin/` removed |
| `make compile-commands` | Regenerate the editor compile database | `compile_commands.json` |
| `make help` | Describe every target | console text |

---

## 3. What happens during a build

```
include/*.h   src/*.cpp
     \          |
      \         |  g++ -c  (compile one at a time)
       +--> build/*.o  <-- also writes build/*.d (which headers were used)
                  |
                  |  g++  (link everything)
                  v
            bin/snake.exe
                  |
                  |  cp glfw3.dll libwinpthread-1.dll bin/
                  v
            bin/ (game is runnable by double-click)
```

### Compilation

Each `.cpp` becomes one `.o` file. A change to `src/Snake.cpp` recompiles only
`build/Snake.o`. The `-MMD -MP` flags also write a `build/Snake.d` listing the
headers that object used, so `make` knows which objects a header edit invalidates.
For example, editing `include/Snake.h` rebuilds three of the six objects —
`Snake.o`, `Game.o`, and `main.o` (which pulls `Snake.h` in through `Game.h`) —
and leaves `Food.o`, `Score.o`, and `Audio.o` alone. Editing `include/Types.h`
rebuilds all five objects that include it; only `Audio.o` is untouched.

### Linking

The objects are combined with the raylib library into a single `bin/snake.exe`,
and the two runtime DLLs are copied next to it.

### Incremental builds

Because of the `.d` files, `make` is safe to run repeatedly: an unchanged tree
compiles nothing. Force a full rebuild with `make clean && make`.

---

## 4. Every flag, and why it is there

### Compile flags

| Flag | Purpose |
| ---- | ------- |
| `-std=c++17` | Language version (needed for `inline constexpr` variables in `Types.h`) |
| `-Wall -Wextra -Wpedantic -Wshadow` | The useful warnings plus strict standard conformance and shadowed-variable detection. This project builds with **zero** — treat any new warning as a defect |
| `-O2` | Optimise for speed; irrelevant to correctness |
| `-MMD -MP` | Emit `.d` dependency files so incremental builds are correct |
| `-Iinclude` | Look for `#include "..."` in `include/`. Without it every include of a project header fails |

### Link flags

| Flag | Purpose |
| ---- | ------- |
| `-mwindows` | Link the GUI subsystem: **no console window** behind the game |
| `-static-libgcc -static-libstdc++` | Bundle the C++ runtime into the exe so it runs on machines without the MSYS2 runtime |
| `-l:libraylib.a` | Link raylib as a **static library**, pulling its code into the exe |
| `-lglfw3` | Link GLFW **dynamically** (see next section) |
| `-lopengl32 -lgdi32 -lwinmm` | Windows libraries raylib needs: OpenGL, GDI, multimedia timers |

---

## 5. Why there are still DLLs in `bin/`

This is the question that trips everybody up.

```
bin/snake.exe  --static-->  contains raylib's code and the C++ runtime
                   |
                   +--imports-->  glfw3.dll           (raylib calls GLFW)
                                  libwinpthread-1.dll (threads used by the MinGW runtime)
```

`libraylib.a` contains raylib's own code, but raylib *calls GLFW*, and GLFW is
referenced through `libglfw3.dll.a` — an import library that resolves to
`glfw3.dll` at runtime. The statically linked C++ runtime in turn still calls the
Windows POSIX-threads implementation, which MSYS2 ships as `libwinpthread-1.dll`.
Both DLLs must sit next to `snake.exe`.

You do not have to trust this — the exe records exactly what it needs:

```bash
objdump -p bin/snake.exe | grep -i "dll name"
```

That is exactly what the last step of the link rule does:

```make
cp -f "$(MSYS2_PREFIX)/bin/$$dll" $(BINDIR)/
```

If you see *"glfw3.dll was not found"*, the game was copied without running
`make`, or it was run from somewhere other than `bin/`. See
[Troubleshooting](#7-troubleshooting).

---

## 6. Using VS Code

| Action | How |
| ------ | --- |
| Build | `Ctrl+Shift+B` (default task) or Terminal > Run Task > **build** |
| Run | Task **run** (`make run`) |
| Test | Task **test** (`make test`) |
| Clean | Task **clean** |
| Debug | **F5** — builds first, then launches under GDB (`bin/snake.exe`, working directory `bin/`) |

**IntelliSense shows red squiggles but the build passes?** The C/C++ extension
has cached an older configuration (from before the headers moved to `include/`,
so it may even reference files that no longer exist). Run
**Developer: Reload Window** (or **C/C++: Reset IntelliSense Database**).

To make that impossible in the first place, the editor reads
`compile_commands.json` — the standard compile database — which records the
exact flags for every translation unit (`"compileCommands"` in
`.vscode/c_cpp_properties.json`). It is tracked in git, and
`make compile-commands` regenerates it after you add a source file or change
`CXXFLAGS`. The compiler remains the source of truth: if `make` succeeds, the
code is correct.

Debugger settings live in `.vscode/launch.json`, tasks in `.vscode/tasks.json`,
and include paths in `.vscode/c_cpp_properties.json`.

---

## 7. Troubleshooting

| Symptom | Cause | Fix |
| ------- | ----- | --- |
| `g++: command not found` | Wrong shell | Open the **MSYS2 UCRT64** shell, then `export PATH=/ucrt64/bin:$PATH` if you launched it from elsewhere |
| `raylib.h: No such file or directory` | Package missing, or shell not UCRT64 | `pacman -S mingw-w64-ucrt-x86_64-raylib`; confirm `ls /ucrt64/include/raylib.h` |
| `'cstddef' file not found` | The include path `-Iinclude` is missing, or the compiler query is stale | Build with `make` (which always passes `-Iinclude`); reload the editor window for the squiggles |
| `undefined reference to 'glfwInit'` / `__imp_glfw...` | Link line missing `-lglfw3` | Check `LDLIBS` in the Makefile still contains `-lglfw3` |
| `make: *** missing separator.  Stop.` | The `Makefile` was checked out with CRLF line endings | The repo ships a `.gitattributes` that forces LF. Run `git add --renormalize .` and re-checkout, or `dos2unix Makefile` |
| `Error: ... glfw3.dll was not found` when starting | Exe copied elsewhere, or not built | Run `make` and start `bin/snake.exe` (the DLLs are copied there automatically) |
| `Permission denied` / `Text file busy` during `make clean` | The game is running | Close `snake.exe` first |
| A black console window opens with the game | Missing `-mwindows` | It must be in `LDFLAGS`, not `LDLIBS` |
| The game opens then closes instantly | An error was written to a console you cannot see | Run it from the shell: `cd bin && ./snake.exe` to read the message |
| No sound | Audio device or driver unavailable | Expected behaviour: `Audio` degrades to a no-op and the game still plays. Check your output device and browser/OS volume exclusions |
| The game window loses focus / does not accept keys | Another window is in front | Click the game window; it needs to be the foreground window for raylib to read keys |
| A header edit produced no rebuild | `build/*.d` were deleted while the `.o` files were kept, so `make` has nothing to work from | `make clean && make` regenerates them; never delete `.d` files on their own |
| Everything recompiles for an unrelated change | Skewed file timestamps (copy, restore, clock change) | `make clean && make` resets the tree |
| High score seems to reset | It is stored per executable | `bin/highscore.txt` — deleting `bin/` with `make clean` removes it by design |
| Antivirus quarantines `snake.exe` | Unsigned new executables | Restore it, or build from source yourself |

---

## 8. Porting notes

The Makefile is written for MSYS2/Windows. The same sources compile elsewhere
with three changes to `LDLIBS`:

- **Linux** (Raylib installed from the distro or built from source):
  `-lraylib -lGL -lm -lpthread -ldl -lrt -lX11`
- **macOS**: `-lraylib -framework OpenGL -framework Cocoa -framework IOKit
  -framework CoreVideo`
- **Windows/MinGW with dynamic raylib**: link `-lraylib.dll.a` instead of
  `-l:libraylib.a`; the DLL copy step must include `raylib.dll`.

Everything else — the `SRCS`/`OBJS` rules, `-MMD -MP` dependency generation, and
the `bin/`/`build/` layout — is portable as written.

---

## 9. Running the tests

```bash
make test
```

```
initial state ok
180 degree rejection ok
queue cap ok
queued turn prediction ok
growth ok
wall collision ok
self collision ok
grid bounds ok
food spawn ok
move interval ok
score ok
ALL LOGIC TESTS PASSED
```

The test binary links `Snake.cpp`, `Food.cpp`, and `Score.cpp` only — no
window, no audio device, no raylib loop. That is possible because the rules live
in the entities rather than in the rendering code. Run them before every
submission; any failed `assert` terminates with a non-zero exit code, so a CI
job can gate on it.

---

## 10. Clean-state checklist

To confirm your environment is healthy from scratch:

```bash
git clone https://github.com/enigmasheet/SnakeGame.git
cd SnakeGame
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make   # once
make help        # shows the target list
make clean && make -j     # builds with zero warnings
make test        # 11 tests pass
ls bin           # snake.exe, glfw3.dll, libwinpthread-1.dll, logic_test.exe
```

If every line above succeeds, the toolchain, the compiler flags, the linker, the
DLL handling, and the tests are all correct.

---

## 11. Continuous integration

`.github/workflows/ci.yml` runs exactly those two commands on every push to
`main` and on every pull request, on `windows-latest` with the same UCRT64
toolchain:

1. `msys2/setup-msys2` installs `gcc`, `raylib`, and `make`
2. `make -j` — the build must stay warning-free
3. `make test` — the 11 assertions must pass

The job is headless by design: the test binary links only entity logic and never
calls `InitWindow()`, so it needs no display. `.gitattributes` force-checks out
the `Makefile` with LF endings, which is what keeps `make` from failing on the
runner with `missing separator`.
