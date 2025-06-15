CXX = g++
CXXFLAGS = -std=c++11 -Wall

SOURCES = main.cpp Utils.cpp PuzzleGame.cpp RSADecryptor.cpp SpaceShooter.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXEC = MultiGame

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXEC) `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

%.o: %.cpp
	$(CXX) $(CXXFLAGS) `sdl2-config --cflags` -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXEC)
