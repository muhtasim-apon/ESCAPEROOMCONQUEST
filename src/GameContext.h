#pragma once
#include <SDL2/SDL.h>

struct GameContext {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    // add any shared data here, e.g. player name, score map, keys collected…
};
