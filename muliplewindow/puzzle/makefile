# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -std=c++17 `sdl2-config --cflags`
LDFLAGS := `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

# Project structure
SRC_DIR := .
BUILD_DIR := build
BIN := main

# Find all .cpp files in the project recursively
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(BIN)

# Linking
$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compilation rule
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build and binary
clean:
	rm -rf $(BUILD_DIR) $(BIN)

# Run the program
run: $(BIN)
	./$(BIN)

.PHONY: all clean run

