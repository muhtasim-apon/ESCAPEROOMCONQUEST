# Compiler and flags
CXX      := g++
CXXFLAGS := -std=c++17 -Wall $(shell sdl2-config --cflags)
PKGFLAGS := $(shell pkg-config --cflags --libs SDL2_image SDL2_ttf SDL2_mixer)
LDFLAGS  := $(shell sdl2-config --libs) $(PKGFLAGS)

# Source discovery
SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:.cpp=.o)

# Executable
TARGET   := mega_escape_puzzle_game

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build then launch from project root (so assets/ is found)
run: all
	@echo "Launching from project root…"
	@cd .. && ./src/$(TARGET)

clean:
	@rm -f $(OBJS) $(TARGET)
	@echo "Cleaned."
