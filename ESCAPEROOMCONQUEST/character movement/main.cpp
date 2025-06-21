#include <SFML/Graphics.hpp>

int main() {
    // Load background texture
    sf::Texture roomTexture;
    if (!roomTexture.loadFromFile("room.png")) {
        return -1;
    }

    // Get background size
    sf::Vector2u bgSize = roomTexture.getSize();

    // Create window same size as background
    sf::RenderWindow window(sf::VideoMode(bgSize.x, bgSize.y), "Robot in Room");
    sf::Sprite roomSprite(roomTexture);

    // Load robot texture
    sf::Texture robotTexture;
    if (!robotTexture.loadFromFile("robot.png")) {
        return -1;
    }
    sf::Sprite robotSprite(robotTexture);
    robotSprite.setPosition(350, 350); // Start in middle

    // Explicit scaling to make robot twice its previous size
    // Old size was 0.075, so new scale is 0.15 (double)
    robotSprite.setScale(0.15f, 0.15f);

    float speed = 6.0f; // Movement speed

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Movement input
        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            movement.y -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            movement.y += speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            movement.x -= speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            movement.x += speed;

        // Predict new position
        sf::FloatRect newPos = robotSprite.getGlobalBounds();
        newPos.left += movement.x;
        newPos.top += movement.y;

        // Check background boundaries
        bool withinBounds = true;
        if (newPos.left < 0 || newPos.top < 0 ||
            newPos.left + newPos.width > bgSize.x ||
            newPos.top + newPos.height > bgSize.y) {
            withinBounds = false;
        }

        // Move only if within boundaries
        if (withinBounds) {
            robotSprite.move(movement);
        }

        // Draw
        window.clear();
        window.draw(roomSprite);
        window.draw(robotSprite);
        window.display();
    }

    return 0;
}
