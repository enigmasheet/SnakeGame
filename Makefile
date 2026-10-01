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

.PHONY: all run test clean

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

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(BUILDDIR)
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

clean:
	rm -rf $(BUILDDIR) $(BINDIR)

-include $(DEPS)
