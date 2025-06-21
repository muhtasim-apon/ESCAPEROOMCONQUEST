#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <iostream>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int PUZZLE_TIME_LIMIT = 30;

struct Puzzle {
    std::string question;
    std::string answer;
};

SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, SDL_Rect& rectOut, int wrapLength = 800) {
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), color, wrapLength);
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    rectOut = {0, 0, surface->w, surface->h};
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 || TTF_Init() < 0 || IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) {
        std::cerr << "Failed to initialize SDL/TTF/IMG\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Deadline Decoder", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("impact.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    SDL_Texture* bgTexture = loadTexture(renderer, "puzzleimage.png");
    SDL_Color white = {255, 255, 255, 255};
    SDL_Event e;

    std::string playerName;
    bool enteringName = true;
    SDL_StartTextInput();

    while (enteringName) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            else if (e.type == SDL_TEXTINPUT) playerName += e.text.text;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) playerName.pop_back();
                else if (e.key.keysym.sym == SDLK_RETURN && !playerName.empty()) enteringName = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect rect;
        SDL_Texture* namePrompt = renderText(renderer, font, "Enter your name to begin:", white, rect);
        rect.x = (SCREEN_WIDTH - rect.w) / 2; rect.y = 250;
        SDL_RenderCopy(renderer, namePrompt, nullptr, &rect);
        SDL_DestroyTexture(namePrompt);

        SDL_Texture* nameText = renderText(renderer, font, playerName, white, rect);
        rect.x = (SCREEN_WIDTH - rect.w) / 2; rect.y = 320;
        SDL_RenderCopy(renderer, nameText, nullptr, &rect);
        SDL_DestroyTexture(nameText);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();

    std::vector<Puzzle> puzzles = {
        {"I have keys but no locks, I have space but no room. What am I?", "keyboard"},
        {"What has to be broken before you use it?", "egg"},
        {"The more you take, the more you leave behind. What am I?", "footsteps"}
    };

    int currentPuzzle = -1;
    std::string userInput;
    bool running = true, puzzleStarted = false, puzzleSolved = false, puzzleFailed = false;
    Uint32 puzzleStartTime = 0;

    SDL_Rect monitorTouchArea = {320, 256, 512, 320};

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (!puzzleStarted && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (mx > monitorTouchArea.x && mx < monitorTouchArea.x + monitorTouchArea.w &&
                    my > monitorTouchArea.y && my < monitorTouchArea.y + monitorTouchArea.h) {
                    currentPuzzle = 0;
                    puzzleStarted = true;
                    puzzleSolved = false;
                    puzzleFailed = false;
                    userInput.clear();
                    puzzleStartTime = SDL_GetTicks();
                }
            }

            if (puzzleStarted && !puzzleSolved && !puzzleFailed && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !userInput.empty()) userInput.pop_back();
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (userInput == puzzles[currentPuzzle].answer) puzzleSolved = true;
                } else {
                    char c = e.key.keysym.sym;
                    if (c >= 32 && c <= 126) userInput += c;
                }
            }

            if ((puzzleSolved || puzzleFailed) && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                currentPuzzle++;
                if (currentPuzzle < static_cast<int>(puzzles.size())) {
                    puzzleStarted = true;
                    puzzleSolved = false;
                    puzzleFailed = false;
                    userInput.clear();
                    puzzleStartTime = SDL_GetTicks();
                } else {
                    running = false;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        int secondsLeft = PUZZLE_TIME_LIMIT - (now - puzzleStartTime) / 1000;
        if (puzzleStarted && !puzzleSolved && secondsLeft <= 0) puzzleFailed = true;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (bgTexture) SDL_RenderCopy(renderer, bgTexture, nullptr, nullptr);

        SDL_Rect rect;
        SDL_Texture* texture = nullptr;

        if (!puzzleStarted) {
            texture = renderText(renderer, font, "Click the screen to start the puzzle...", white, rect);
            rect.x = (SCREEN_WIDTH - rect.w) / 2; rect.y = SCREEN_HEIGHT - 100;
        } else if (puzzleSolved) {
            texture = renderText(renderer, font, "Correct! Press SPACE for next puzzle.", white, rect);
            rect.x = (SCREEN_WIDTH - rect.w) / 2; rect.y = 100;
        } else if (puzzleFailed) {
            texture = renderText(renderer, font, "Time's up! Press SPACE to try next puzzle.", white, rect);
            rect.x = (SCREEN_WIDTH - rect.w) / 2; rect.y = 100;
        } else {
            std::string fullText = puzzles[currentPuzzle].question + "\n\nYour Answer: " + userInput + "\n\nTime Left: " + std::to_string(secondsLeft);
            texture = renderText(renderer, font, fullText, white, rect);
            rect.x = (SCREEN_WIDTH - rect.w) / 2; rect.y = 100;
        }

        if (texture) {
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
            SDL_DestroyTexture(texture);
        }

        SDL_Texture* welcomeTex = renderText(renderer, font, "Welcome, " + playerName + "!", white, rect);
        rect.x = SCREEN_WIDTH - rect.w - 20;
        rect.y = 20;
        SDL_RenderCopy(renderer, welcomeTex, nullptr, &rect);
        SDL_DestroyTexture(welcomeTex);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Show decryptor image in new window after game is completed
    SDL_Window* winWindow = SDL_CreateWindow("Decryptor Unlocked", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* winRenderer = SDL_CreateRenderer(winWindow, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* winImage = loadTexture(winRenderer, "decryptorimage.png");

    bool showing = true;
    while (showing) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN) {
                showing = false;
            }
        }

        SDL_SetRenderDrawColor(winRenderer, 0, 0, 0, 255);
        SDL_RenderClear(winRenderer);
        if (winImage) SDL_RenderCopy(winRenderer, winImage, nullptr, nullptr);
        SDL_RenderPresent(winRenderer);
    }

    SDL_DestroyTexture(winImage);
    SDL_DestroyRenderer(winRenderer);
    SDL_DestroyWindow(winWindow);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
