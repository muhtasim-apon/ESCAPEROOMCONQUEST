#include "InterfaceModule.h"
#include "Utils.h"
#include "MapModule.h"
#include "GameContext.h"
#include "GameState.h"
#include "input.h"
#include "button.h"
#include "Leaderboard.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

int main(int argc, char* argv[]) {
    // Initialize SDL and related libraries
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Log("IMG_Init Error: %s", IMG_GetError());
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init Error: %s", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow("Mega Escape-Puzzle Game",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1024, 768, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // GameContext setup
    GameContext ctx;
    ctx.window = window;
    ctx.renderer = renderer;

    // Main game loop
    GameState state = GameState::INTERFACE;
    while (state != GameState::EXIT) {
        switch (state) {
            case GameState::INTERFACE:
                state = InterfaceModule::run(ctx);
                break;
            // Add other cases like Map, Puzzle, etc., here as per your game design
            // e.g.
             case GameState::MAP:
                state = MapModule::run(ctx);
             break;
            // case GameState::PUZZLE:
            //     state = PuzzleGame::run(ctx);
            //     break;
            // etc.
            default:
                state = GameState::EXIT;
                break;
        }
    }

    // Cleanup and quit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}