#include "PuzzleGame.h"
#include "Utils.h"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int PUZZLE_TIME_LIMIT = 30;

SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        cerr << "Failed to load image: " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void runPuzzle(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Texture* bgTexture = loadTexture(renderer, "assets/puzzleimage.png");
    SDL_Color white = {255, 255, 255, 255};
    SDL_Event e;

    string playerName;
    bool enteringName = true;
    SDL_StartTextInput();

    while (enteringName) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return;
            else if (e.type == SDL_TEXTINPUT) playerName += e.text.text;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) playerName.pop_back();
                else if (e.key.keysym.sym == SDLK_RETURN && !playerName.empty()) enteringName = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderText(renderer, font, "Enter your name to begin:", white, (SCREEN_WIDTH / 2) - 150, 250);
        renderText(renderer, font, playerName + "_", white, (SCREEN_WIDTH / 2) - 150, 320);
        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();

    struct Puzzle {
        string question;
        string answer;
    };

    vector<Puzzle> puzzles = {
        {"I have keys but no locks, I have space but no room. What am I?", "keyboard"},
        {"What has to be broken before you use it?", "egg"},
        {"The more you take, the more you leave behind. What am I?", "footsteps"}
    };

    int currentPuzzle = -1;
    string userInput;
    bool running = true, puzzleStarted = false, puzzleSolved = false, puzzleFailed = false;
    Uint32 puzzleStartTime = 0;
    SDL_Rect monitorTouchArea = {320, 256, 512, 320};

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return;

            if (!puzzleStarted && e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;
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
                if (currentPuzzle < (int)puzzles.size()) {
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

        if (!puzzleStarted) {
            renderText(renderer, font, "Click the screen to start the puzzle...", white, (SCREEN_WIDTH / 2) - 250, SCREEN_HEIGHT - 100);
        } else if (puzzleSolved) {
            renderText(renderer, font, "Correct! Press SPACE for next puzzle.", white, (SCREEN_WIDTH / 2) - 250, 100);
        } else if (puzzleFailed) {
            renderText(renderer, font, "Time's up! Press SPACE to try next puzzle.", white, (SCREEN_WIDTH / 2) - 250, 100);
        } else {
            renderText(renderer, font, puzzles[currentPuzzle].question, white, 100, 100);
            renderText(renderer, font, "Your Answer: " + userInput, white, 100, 200);
            renderText(renderer, font, "Time Left: " + to_string(secondsLeft), white, 100, 300);
        }

        renderText(renderer, font, "Welcome, " + playerName + "!", white, SCREEN_WIDTH - 300, 20);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(bgTexture);

    // ✅ Show decryptor unlocked screen
    SDL_Surface* decryptorSurf = IMG_Load("assets/decryptor.png");
    if (!decryptorSurf) {
        std::cerr << "Failed to load decryptor.png: " << IMG_GetError() << std::endl;
    } else {
        SDL_Texture* decryptorTex = SDL_CreateTextureFromSurface(renderer, decryptorSurf);
        SDL_FreeSurface(decryptorSurf);

        if (!decryptorTex) {
            std::cerr << "Failed to create decryptor texture: " << SDL_GetError() << std::endl;
        } else {
            bool showing = true;
            while (showing) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
                        showing = false;
                    }
                }
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, decryptorTex, nullptr, nullptr);
                SDL_RenderPresent(renderer);
            }
            SDL_DestroyTexture(decryptorTex);
        }
    }
}
