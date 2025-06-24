#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

// Function to render UTF-8 text onto the current render target
void renderText(SDL_Renderer* renderer,
                TTF_Font* font,
                const std::string& text,
                SDL_Color color,
                int x,
                int y);

// Function to load a texture from a file
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filePath);
