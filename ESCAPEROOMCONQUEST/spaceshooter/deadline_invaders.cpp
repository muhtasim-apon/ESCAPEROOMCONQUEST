#include <SDL2/SDL.h> // Include the main SDL library for window, rendering, and events.
#include <SDL2/SDL_image.h> // Include SDL_image for loading various image formats (like PNG).
#include <SDL2/SDL_ttf.h> // Include SDL_ttf for rendering TrueType fonts.
#include <iostream> // Include iostream for standard input/output operations (e.g., error messages).
#include <vector> // Include vector for dynamic arrays to manage bullets and enemies.
#include <algorithm> // Include algorithm for functions like std::remove_if to clean up vectors.
#include <ctime> // Include ctime for time-related functions, used to seed the random number generator.
#include <cmath> // Include cmath for mathematical functions like sin, used for animation.

// Define constants for screen dimensions and game winning score.
const int SCREEN_WIDTH = 800; // Define the width of the game window in pixels.
const int SCREEN_HEIGHT = 600; // Define the height of the game window in pixels.
const int WIN_SCORE = 200; // Define the score required for the player to win the game.

// Structure to represent a Bullet in the game.
struct Bullet {
    SDL_Rect rect; // SDL_Rect holds the position (x, y) and dimensions (width, height) of the bullet.
    int speed = -10; // The vertical speed of the bullet. Negative means it moves upwards.
};

// Structure to represent an Enemy in the game.
struct Enemy {
    SDL_Rect rect; // SDL_Rect holds the position (x, y) and dimensions (width, height) of the enemy.
    std::string label; // A text label (e.g., "PROJECT", "QUIZ") displayed on the enemy.
    int speed = 1; // The vertical speed of the enemy. Positive means it moves downwards.
};

// Function to check for collision between two SDL_Rect objects.
// Returns true if the rectangles intersect, false otherwise.
bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return SDL_HasIntersection(&a, &b); // Uses an SDL function to efficiently check for intersection.
}

// Function to render text on the SDL renderer.
// renderer: The SDL_Renderer to draw on.
// font: The TTF_Font to use for rendering the text.
// text: The string to be rendered.
// color: The SDL_Color of the text.
// x, y: The top-left coordinates where the text will be drawn.
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    // Render the text onto an SDL_Surface. TTF_RenderText_Blended provides anti-aliased text.
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) { // Check if surface creation failed.
        std::cerr << "TTF_RenderText_Blended error: " << TTF_GetError() << "\n"; // Print error to console.
        return; // Exit if surface is null.
    }
    // Create an SDL_Texture from the surface. Textures are optimized for GPU rendering.
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) { // Check if texture creation failed.
        std::cerr << "SDL_CreateTextureFromSurface error: " << SDL_GetError() << "\n"; // Print error to console.
        SDL_FreeSurface(surface); // Free the surface even if texture creation failed.
        return; // Exit if texture is null.
    }
    // Define the destination rectangle for the texture on the renderer.
    SDL_Rect dst = {x, y, surface->w, surface->h};
    // Copy the texture to the renderer at the specified destination. NULL means copy the entire texture.
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    // Free the SDL_Surface as it's no longer needed after creating the texture.
    SDL_FreeSurface(surface);
    // Destroy the SDL_Texture to free GPU memory.
    SDL_DestroyTexture(texture);
}

// Function to get the player's name before starting the game.
// renderer: The SDL_Renderer to draw on.
// font: The TTF_Font to use for rendering text.
// Returns the entered player's name as a string.
std::string getPlayerName(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_StartTextInput(); // Enable text input events from the keyboard.
    std::string name = ""; // Initialize an empty string to store the player's name.
    SDL_Color white = {255, 255, 255}; // Define a white color for the text.
    SDL_Event e; // Declare an SDL_Event variable to handle events.
    bool done = false; // Flag to control the input loop.

    while (!done) { // Loop until the player finishes entering their name.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set the drawing color to black.
        SDL_RenderClear(renderer); // Clear the renderer with the black color.
        renderText(renderer, font, "Enter Your Name:", white, 250, 200); // Render the prompt text.
        renderText(renderer, font, name + "_", white, 250, 250); // Render the current name with a blinking cursor.
        SDL_RenderPresent(renderer); // Update the screen to show the rendered text.

        while (SDL_PollEvent(&e)) { // Poll for pending SDL events.
            if (e.type == SDL_QUIT) return "Player"; // If the window close button is clicked, return a default name.
            if (e.type == SDL_TEXTINPUT) name += e.text.text; // If text is input, append it to the name string.
            if (e.type == SDL_KEYDOWN) { // If a key is pressed.
                // If Backspace is pressed and the name is not empty, remove the last character.
                if (e.key.keysym.sym == SDLK_BACKSPACE && !name.empty()) name.pop_back();
                // If Enter (Return) is pressed and the name is not empty, set done to true to exit the loop.
                if (e.key.keysym.sym == SDLK_RETURN && !name.empty()) done = true;
            }
        }
    }

    SDL_StopTextInput(); // Disable text input events.
    return name; // Return the entered player's name.
}

// Function to generate a random "encrypted code" (for game flavor).
// Returns a string representing the encrypted code.
std::string generateEncryptedCode() {
    std::string code = "Encrypted code: "; // Start with a prefix.
    for (int i = 0; i < 16; ++i) { // Generate 16 random characters.
        char letter = 'A' + rand() % 26; // Generate a random uppercase letter (A-Z).
        code += letter; // Append the letter to the code string.
    }
    return code; // Return the generated code.
}

// Function to display the end game screen (win or lose).
// renderer: The SDL_Renderer to draw on.
// font: The TTF_Font to use for rendering text.
// name: The player's name.
// score: The player's final score.
// won: A boolean indicating whether the player won (true) or lost (false).
void showEndScreen(SDL_Renderer* renderer, TTF_Font* font, const std::string& name, int score, bool won) {
    SDL_Color white = {255, 255, 255}; // White color.
    SDL_Color green = {0, 255, 0}; // Green color for win message.
    SDL_Color red = {255, 0, 0}; // Red color for lose message.

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set drawing color to black.
    SDL_RenderClear(renderer); // Clear the screen to black.

    renderText(renderer, font, "Game Over!", white, 320, 180); // Render "Game Over!" title.
    renderText(renderer, font, "Player: " + name, white, 300, 230); // Render player's name.
    renderText(renderer, font, "Score: " + std::to_string(score), white, 300, 270); // Render player's score.

    if (won) { // If the player won.
        std::string encrypted = generateEncryptedCode(); // Generate the "encrypted code".
        renderText(renderer, font, encrypted, green, 220, 310); // Display the encrypted code in green.
    } else { // If the player lost.
        renderText(renderer, font, "Try Again!", red, 300, 310); // Display "Try Again!" in red.
    }

    SDL_RenderPresent(renderer); // Update the screen to show the end screen.
    SDL_Delay(5000); // Pause for 5 seconds before returning (and exiting main).
}

// Main function where the program execution begins.
int main() {
    srand(time(NULL)); // Seed the random number generator with the current time for varied random numbers.

    // Initialize SDL subsystems. If any fails, print an error and exit.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { // Initialize SDL's video subsystem.
        std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
        return 1;
    }
    if (TTF_Init() < 0) { // Initialize SDL_ttf library.
        std::cerr << "TTF_Init error: " << TTF_GetError() << "\n";
        SDL_Quit(); // Quit SDL if TTF fails.
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) { // Initialize SDL_image for PNG support.
        std::cerr << "IMG_Init error: " << IMG_GetError() << "\n";
        TTF_Quit(); // Quit TTF if IMG fails.
        SDL_Quit(); // Quit SDL if IMG fails.
        return 1;
    }

    // Create the main game window.
    SDL_Window* window = SDL_CreateWindow(
        "Deadline Invaders", // Window title.
        SDL_WINDOWPOS_CENTERED, // Center window horizontally.
        SDL_WINDOWPOS_CENTERED, // Center window vertically.
        SCREEN_WIDTH, // Window width.
        SCREEN_HEIGHT, // Window height.
        0 // No special flags.
    );
    if (!window) { // Check if window creation failed.
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
        TTF_Quit(); IMG_Quit(); SDL_Quit(); // Clean up all initialized subsystems.
        return 1;
    }

    // Create a hardware-accelerated renderer for drawing.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { // Check if renderer creation failed.
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window); // Destroy window if renderer fails.
        TTF_Quit(); IMG_Quit(); SDL_Quit();
        return 1;
    }

    // Load the font for text rendering.
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) { // Check if font loading failed.
        std::cerr << "Font load error: " << TTF_GetError() << "\n";
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); // Clean up.
        TTF_Quit(); IMG_Quit(); SDL_Quit();
        return 1;
    }

    // Get the player's name before starting the main game loop.
    std::string playerName = getPlayerName(renderer, font);

    // Load and create texture for the background image.
    SDL_Surface* bgSurface = IMG_Load("space_background.png");
    if (!bgSurface) { // Check if background image loading failed.
        std::cerr << "Background load error: " << IMG_GetError() << "\n";
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); TTF_CloseFont(font); // Clean up.
        TTF_Quit(); IMG_Quit(); SDL_Quit();
        return 1;
    }
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface); // Free the surface after creating the texture.

    // Load and create texture for the enemy ship.
    SDL_Surface* shipSurface = IMG_Load("ship2.png");
    if (!shipSurface) { // Check if enemy ship image loading failed.
        std::cerr << "ship2.png load error: " << IMG_GetError() << "\n";
        SDL_DestroyTexture(bgTexture); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); TTF_CloseFont(font); // Clean up.
        TTF_Quit(); IMG_Quit(); SDL_Quit();
        return 1;
    }
    SDL_Texture* enemyTex = SDL_CreateTextureFromSurface(renderer, shipSurface);
    SDL_FreeSurface(shipSurface); // Free the surface after creating the texture.

    // Load and create texture for the player ship.
    SDL_Surface* playerSurf = IMG_Load("ship1.png");
    if (!playerSurf) { // Check if player ship image loading failed.
        std::cerr << "ship1.png load error: " << IMG_GetError() << "\n";
        SDL_DestroyTexture(enemyTex); SDL_DestroyTexture(bgTexture); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); TTF_CloseFont(font); // Clean up.
        TTF_Quit(); IMG_Quit(); SDL_Quit();
        return 1;
    }
    SDL_Texture* playerTex = SDL_CreateTextureFromSurface(renderer, playerSurf);
    SDL_FreeSurface(playerSurf); // Free the surface after creating the texture.

    // Initialize player's position and size.
    SDL_Rect player = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 40};
    std::vector<Bullet> bullets; // Vector to store active bullets.
    std::vector<Enemy> enemies; // Vector to store active enemies.
    // Array of labels for enemies.
    std::string labels[] = {"PROJECT", "QUIZ", "LAB", "EXAM"};

    int score = 0; // Initialize player's score.
    bool quit = false; // Flag to control the main game loop.
    SDL_Event e; // Event variable for handling input.
    Uint32 lastSpawnTime = SDL_GetTicks(); // Stores the time when the last enemy was spawned.

    // Main game loop. This loop runs continuously until the 'quit' flag is true.
    while (!quit) {
        // Event handling loop: Process all pending SDL events.
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true; // If the user clicks the window close button, set quit to true.
            // If a key is pressed and it's the Spacebar.
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                // Add a new bullet to the bullets vector.
                // Bullet spawns from the center top of the player ship.
                bullets.push_back({{player.x + player.w / 2 - 5, player.y, 10, 20}});
            }
        }

        // Get the current state of the keyboard for continuous movement.
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        // If Left arrow key is pressed and player is not at the left edge, move left.
        if (keys[SDL_SCANCODE_LEFT] && player.x > 0) player.x -= 7;
        // If Right arrow key is pressed and player is not at the right edge, move right.
        if (keys[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - player.w) player.x += 7;

        // Update bullet positions.
        for (auto& b : bullets) b.rect.y += b.speed;
        // Remove bullets that have moved off the top of the screen.
        // std::remove_if moves elements to be removed to the end, then erase removes them.
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b){ return b.rect.y < 0; }), bullets.end());

        // Enemy spawning logic.
        Uint32 current = SDL_GetTicks(); // Get current time in milliseconds.
        if (current - lastSpawnTime > 1000) { // If 1 second (1000 ms) has passed since last spawn.
            Enemy newEnemy; // Create a new enemy object.
            // Set random x position for the enemy, ensuring it stays within screen bounds.
            newEnemy.rect = {rand() % (SCREEN_WIDTH - 60), 0, 60, 40};
            newEnemy.label = labels[rand() % 4]; // Assign a random label from the labels array.
            newEnemy.speed = 2 + rand() % 3; // Assign a random speed between 2 and 4.
            enemies.push_back(newEnemy); // Add the new enemy to the enemies vector.
            lastSpawnTime = current; // Update the last spawn time.
        }

        // Update enemy positions.
        for (auto& en : enemies) en.rect.y += en.speed;

        // Collision detection between bullets and enemies.
        for (size_t i = 0; i < bullets.size(); ++i) { // Iterate through each bullet.
            for (size_t j = 0; j < enemies.size(); ++j) { // Iterate through each enemy.
                if (checkCollision(bullets[i].rect, enemies[j].rect)) { // If a collision occurs.
                    bullets.erase(bullets.begin() + i); // Remove the hit bullet.
                    enemies.erase(enemies.begin() + j); // Remove the hit enemy.
                    score += 10; // Increase score.
                    break; // Break from inner loop as the current bullet is destroyed.
                }
            }
        }

        // Check if any enemy has reached the bottom of the screen (game over condition).
        for (auto& en : enemies) {
            if (en.rect.y > SCREEN_HEIGHT) {
                quit = true; // Set quit to true to end the game.
                break; // Exit loop immediately if an enemy passes.
            }
        }

        // Check if the player has reached the winning score.
        if (score >= WIN_SCORE) quit = true; // If winning score is reached, set quit to true.

        // --- Rendering Section ---
        SDL_RenderClear(renderer); // Clear the entire renderer with the current drawing color (usually black).
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL); // Draw the background texture, stretching it to fill the screen.
        SDL_RenderCopy(renderer, playerTex, NULL, &player); // Draw the player ship at its current position.

        // Animate text glow for enemy labels using a sine wave.
        // Value oscillates between 1 and 255 for a pulsating effect.
        int glow = static_cast<int>(128 + 127 * sin(SDL_GetTicks() / 300.0));
        SDL_Color glowColor = {
            static_cast<Uint8>(glow), // Red component.
            static_cast<Uint8>(glow), // Green component.
            static_cast<Uint8>(glow)  // Blue component.
        };

        // Draw enemies and their labels.
        for (auto& en : enemies) {
            SDL_RenderCopy(renderer, enemyTex, NULL, &en.rect); // Draw the enemy ship image.
            // Draw the enemy's label slightly offset from its rectangle, with the glowing color.
            renderText(renderer, font, en.label, glowColor, en.rect.x + 5, en.rect.y + 10);
        }

        SDL_Color white = {255, 255, 255}; // Define white color for general text.
        // Draw bullets as filled yellow rectangles.
        for (auto& b : bullets) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Set drawing color to yellow (R, G, B, A).
            SDL_RenderFillRect(renderer, &b.rect); // Fill the bullet's rectangle with yellow.
        }

        // Render the current score in the top-left corner.
        renderText(renderer, font, "Score: " + std::to_string(score), white, 10, 10);
        SDL_RenderPresent(renderer); // Present the rendered frame to the screen (swap buffers).
        SDL_Delay(16); // Introduce a small delay to cap the frame rate (approx. 60 FPS).
    }

    // After the main game loop ends, determine if the player won or lost.
    bool won = score >= WIN_SCORE;
    // Show the appropriate end screen.
    showEndScreen(renderer, font, playerName, score, won);

    // --- Cleanup Section ---
    // Destroy all loaded textures to free GPU memory.
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(enemyTex);
    // Destroy the renderer.
    SDL_DestroyRenderer(renderer);
    // Destroy the window.
    SDL_DestroyWindow(window);
    // Close the loaded font.
    TTF_CloseFont(font);
    // Quit SDL_ttf subsystem.
    TTF_Quit();
    // Quit SDL_image subsystem.
    IMG_Quit();
    // Quit main SDL subsystem.
    SDL_Quit();
    return 0; // Indicate successful program execution.
}