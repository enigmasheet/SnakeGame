CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP -Iinclude
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

.PHONY: all run test clean help

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

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

-include $(DEPS)
