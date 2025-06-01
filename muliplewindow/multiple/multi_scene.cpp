#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <string>

const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_SCENES = 5; // or adjust based on how many images you have

bool fadeTransition(SDL_Renderer* renderer, SDL_Texture* nextTexture) {
    for (int alpha = 255; alpha >= 0; alpha -= 15) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, nextTexture, NULL, NULL);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
        SDL_Rect fade = {0, 0, WIDTH, HEIGHT};
        SDL_RenderFillRect(renderer, &fade);
        SDL_RenderPresent(renderer);
        SDL_Delay(15);
    }
    return true;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image " << path << ": " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "Initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Image Window Switcher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::vector<std::string> imagePaths = {
        "scene1.png",
        "scene2.png",
        "scene3.png",
        "scene4.png",
        "scene5.png"
    };

    std::vector<SDL_Texture*> scenes;
    for (const auto& path : imagePaths) {
        SDL_Texture* tex = loadTexture(renderer, path);
        if (!tex) {
            SDL_Quit();
            return 1;
        }
        scenes.push_back(tex);
    }

    int currentScene = 0;
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;

            if (e.type == SDL_KEYDOWN) {
                int newScene = currentScene;
                if (e.key.keysym.sym == SDLK_1) newScene = 0;
                if (e.key.keysym.sym == SDLK_2 && scenes.size() > 1) newScene = 1;
                if (e.key.keysym.sym == SDLK_3 && scenes.size() > 2) newScene = 2;
                if (e.key.keysym.sym == SDLK_4 && scenes.size() > 3) newScene = 3;
                if (e.key.keysym.sym == SDLK_5 && scenes.size() > 4) newScene = 4;

                if (newScene != currentScene) {
                    fadeTransition(renderer, scenes[newScene]);
                    currentScene = newScene;
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scenes[currentScene], NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    for (auto& tex : scenes)
        SDL_DestroyTexture(tex);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
