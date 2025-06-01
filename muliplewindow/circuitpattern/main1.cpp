#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

const int TILE_SIZE = 100;
const int ROWS = 6;
const int COLS = 6;
const float TIME_LIMIT = 60.0f; // 1 minute

enum TileType { EMPTY, START, END, RESISTOR, WIRE, DIODE, CAPACITOR, BATTERY };

struct Tile {
    TileType type;
    sf::RectangleShape shape;
    sf::Text label;
    bool visited = false;
};

std::vector<sf::Vector2i> validPath = {
    {0,0}, {0,1}, {0,2}, {0,3}, {1,2}, {2,2}, {3,2}, {3,3}, {3,4}, {3,5}
};

bool isValidStep(int index, sf::Vector2i pos) {
    return index < validPath.size() && validPath[index] == pos;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(COLS * TILE_SIZE, ROWS * TILE_SIZE + 50), "Circuit Maze Game");

    // Load font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Font loading failed!\n";
        return -1;
    }

    // Load PCB background
   // Load PCB background
sf::Texture backgroundTexture;
if (!backgroundTexture.loadFromFile("background.png")) {
    std::cerr << "Background image loading failed!\n";
    return -1;
}

    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(
        float(COLS * TILE_SIZE) / backgroundTexture.getSize().x,
        float(ROWS * TILE_SIZE) / backgroundTexture.getSize().y
    );

    // Grid
    Tile grid[ROWS][COLS];
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            grid[y][x].type = EMPTY;
            grid[y][x].shape.setSize(sf::Vector2f(TILE_SIZE - 2, TILE_SIZE - 2));
            grid[y][x].shape.setFillColor(sf::Color::Transparent);
            grid[y][x].shape.setOutlineColor(sf::Color::White);
            grid[y][x].shape.setOutlineThickness(1);
            grid[y][x].shape.setPosition(x * TILE_SIZE, y * TILE_SIZE);

            grid[y][x].label.setFont(font);
            grid[y][x].label.setCharacterSize(20);
            grid[y][x].label.setFillColor(sf::Color::White);
            grid[y][x].label.setPosition(x * TILE_SIZE + 10, y * TILE_SIZE + 35);
        }
    }

    std::vector<std::pair<sf::Vector2i, std::string>> components = {
        {{0,0}, "S"}, {{0,1}, "R"}, {{0,2}, "W"}, {{0,3}, "D"},
        {{1,2}, "W"}, {{2,2}, "C"}, {{3,2}, "B"},
        {{3,3}, "W"}, {{3,4}, "W"}, {{3,5}, "E"}
    };

    for (auto& comp : components) {
        int y = comp.first.y;
        int x = comp.first.x;
        std::string label = comp.second;

        grid[y][x].label.setString(label);
        if (label == "S") grid[y][x].type = START;
        else if (label == "E") grid[y][x].type = END;
        else if (label == "R") grid[y][x].type = RESISTOR;
        else if (label == "C") grid[y][x].type = CAPACITOR;
        else if (label == "D") grid[y][x].type = DIODE;
        else if (label == "B") grid[y][x].type = BATTERY;
        else if (label == "W") grid[y][x].type = WIRE;
    }

    int pathIndex = 0;
    bool gameWon = false, gameLost = false;

    sf::Clock clock;
    sf::Text timerText;
    timerText.setFont(font);
    timerText.setCharacterSize(20);
    timerText.setFillColor(sf::Color::Yellow);
    timerText.setPosition(10, ROWS * TILE_SIZE + 10);

    sf::Text resultText;
    resultText.setFont(font);
    resultText.setCharacterSize(24);
    resultText.setFillColor(sf::Color::White);
    resultText.setPosition(200, ROWS * TILE_SIZE + 10);

    while (window.isOpen()) {
        sf::Event event;
        float elapsed = clock.getElapsedTime().asSeconds();

        timerText.setString("Time Left: " + std::to_string(int(TIME_LIMIT - elapsed)));

        if (elapsed >= TIME_LIMIT && !gameWon) {
            gameLost = true;
            resultText.setString("Time's up! You lost.");
        }

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!gameWon && !gameLost && event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                int x = mousePos.x / TILE_SIZE;
                int y = mousePos.y / TILE_SIZE;

                if (x < COLS && y < ROWS) {
                    if (isValidStep(pathIndex, {y, x})) {
                        grid[y][x].visited = true;
                        grid[y][x].shape.setFillColor(sf::Color(0, 255, 0, 100));
                        pathIndex++;
                        if (pathIndex == validPath.size()) {
                            gameWon = true;
                            resultText.setString("Success! You completed the circuit.");
                        }
                    } else {
                        grid[y][x].shape.setFillColor(sf::Color(255, 0, 0, 100));
                        gameLost = true;
                        resultText.setString("Wrong step! You lost.");
                    }
                }
            }
        }

        window.clear();
        window.draw(backgroundSprite);

        // Draw grid
        for (int y = 0; y < ROWS; ++y)
            for (int x = 0; x < COLS; ++x) {
                window.draw(grid[y][x].shape);
                window.draw(grid[y][x].label);
            }

        window.draw(timerText);
        if (gameWon || gameLost)
            window.draw(resultText);

        window.display();
    }

    return 0;
}
