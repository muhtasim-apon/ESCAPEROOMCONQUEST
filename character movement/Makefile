# Makefile

# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -Wall -std=c++17
# SFML libraries
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Executable name
TARGET = robot_room

# Source files
SRC = main.cpp

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(SFML_LIBS)

# Clean up
clean:
	rm -f $(TARGET)
