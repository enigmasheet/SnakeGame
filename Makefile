CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -O2 -MMD -MP -Iinclude
LDFLAGS  := -mwindows -static-libgcc -static-libstdc++
LDLIBS   := -l:libraylib.a -lglfw3 -lopengl32 -lgdi32 -lwinmm

MSYS2_PREFIX  ?= C:/msys64/ucrt64
RUNTIME_DLLS  := glfw3.dll libwinpthread-1.dll

SRCDIR   := src
INCDIR   := include
TESTDIR  := tests
BUILDDIR := build
BINDIR   := bin

TARGET   := $(BINDIR)/snake.exe

SRCS    := $(wildcard $(SRCDIR)/*.cpp)
OBJS    := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
DEPS    := $(OBJS:.o=.d)
HEADERS := $(wildcard $(INCDIR)/*.h)

.PHONY: all run test clean help compile-commands

# Make runs this when invoked with no target.
.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@for dll in $(RUNTIME_DLLS); do \
		if [ -f "$(MSYS2_PREFIX)/bin/$$dll" ]; then \
			cp -f "$(MSYS2_PREFIX)/bin/$$dll" $(BINDIR)/; \
		else \
			echo "warning: $$dll not found, snake.exe may fail to start"; \
		fi; \
	done

# Per-object dependencies come from the .d files written by -MMD (-include below).
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

TEST_TARGET := $(BINDIR)/logic_test.exe
TEST_SRCS   := $(TESTDIR)/logic_test.cpp $(SRCDIR)/Snake.cpp $(SRCDIR)/Food.cpp $(SRCDIR)/Score.cpp

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) $(HEADERS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

help:
	@echo "Snake - make targets:"
	@echo "  make (or make all)   build everything into bin/"
	@echo "  make run             build, then start the game"
	@echo "  make test            build and run the headless logic tests"
	@echo "  make clean           delete the generated build/ and bin/ folders"
	@echo "  make compile-commands  regenerate compile_commands.json for editors"
	@echo "  make help            show this message"
	@echo ""
	@echo "Output layout:"
	@echo "  build/   object and dependency files (never edit these)"
	@echo "  bin/     snake.exe, runtime DLLs, logic_test.exe, highscore.txt"
	@echo ""
	@echo "Files:"
	@echo "  include/ public headers (the interfaces, documented with Doxygen)"
	@echo "  src/     implementations"
	@echo "  tests/   logic tests (run them with make test)"

COMPILE_DB := compile_commands.json
# Windows-style project root for the compile database. -a makes cygpath resolve
# a relative argument ("." would otherwise come back as "."), -m keeps mixed
# slashes; falls back to pwd on systems without cygpath.
DB_ROOT    := $(shell cygpath -am . 2>/dev/null || pwd)

# Writes the standard compile database: one entry per translation unit with the
# exact flags used, so VS Code / clangd resolve includes without guessing.
# Run after adding a source file or changing CXXFLAGS.
compile-commands:
	@printf '[\n' > $(COMPILE_DB)
	@first=1; for src in $(SRCS) $(TESTDIR)/logic_test.cpp; do \
		if [ $$first -eq 1 ]; then first=0; else printf ',\n' >> $(COMPILE_DB); fi; \
		obj=$$(basename $$src .cpp); \
		printf '  {"directory": "%s", "file": "%s", "command": "%s %s -c %s -o build/%s.o"}' \
			"$(DB_ROOT)" "$$src" "$(CXX)" "$(CXXFLAGS)" "$$src" "$$obj" >> $(COMPILE_DB); \
	done
	@printf '\n]\n' >> $(COMPILE_DB)
	@echo "wrote $(COMPILE_DB) ($$(grep -c '"file"' $(COMPILE_DB)) entries)"

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

-include $(DEPS)
