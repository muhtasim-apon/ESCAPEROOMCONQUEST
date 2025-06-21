// src/Leaderboard.h
#pragma once
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "GameState.h"

struct LeaderboardEntry {
    std::string name;
    float       time;
};

namespace Leaderboard {
    void loadFromFile(const std::string& filename);
    GameState run(SDL_Renderer* ren, TTF_Font* font);
}
