#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Blocks until the user types a non‐empty name + Enter.
// Returns true and fills `outName` on success, false if they closed the window.
bool getPlayerName(SDL_Renderer* ren, std::string& outName);
