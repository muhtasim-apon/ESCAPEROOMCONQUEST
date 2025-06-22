// src/InterfaceModule.cpp
#include "InterfaceModule.h"
#include "GameState.h"
#include "GameContext.h"
#include "Utils.h"
#include "input.h"
#include "button.h"
#include "Leaderboard.h"
#include "PuzzleModule.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

namespace {
    SDL_Texture*       bgTex   = nullptr;
    TTF_Font*          font    = nullptr;
    std::vector<Button> buttons;

    void initUI(SDL_Renderer* ren) {
        if (!bgTex) {
            bgTex = loadTexture(ren, "assets/menuback.png");
        }
        if (!font) {
            font = TTF_OpenFont("assets/arial.ttf", 36);
            TTF_SetFontStyle(font, TTF_STYLE_BOLD);
        }
        if (buttons.empty()) {
            buttons.emplace_back(100, 200, 300, 60, "New Game",    SDL_Color{255,255,255,200});
            buttons.emplace_back(100, 280, 300, 60, "Map",         SDL_Color{255,255,255,200});
            buttons.emplace_back(100, 360, 300, 60, "Leaderboard", SDL_Color{255,255,255,200});
            buttons.emplace_back(100, 440, 300, 60, "Exit",        SDL_Color{255,255,255,200});
        }
    }
}

GameState InterfaceModule::run(GameContext& ctx) {
    SDL_Renderer* ren = ctx.renderer;
    initUI(ren);

    bool loop = true;
    GameState next = GameState::EXIT;
    SDL_Event e;

    while (loop) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        for (auto& b : buttons) {
            b.isHovered = (mx >= b.rect.x && mx <= b.rect.x + b.rect.w
                         && my >= b.rect.y && my <= b.rect.y + b.rect.h);
        }

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                next = GameState::EXIT;
                loop = false;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                for (auto& b : buttons) {
                    if (!b.isHovered) continue;
                    if (b.label == "New Game") {
                        std::string name;
                        if (getPlayerName(ren, name)) {
                            next = GameState::MAP;
                        }
                    }
                    else if (b.label == "Map") {
                        next = GameState::MAP;
                    }
                    else if (b.label == "Leaderboard") {
                        Leaderboard::loadFromFile("assets/leaderboard.txt");
                        Leaderboard::run(ren, font);
                    }
                    else if (b.label == "Exit") {
                        next = GameState::EXIT;
                    }
                    loop = false;
                    break;
                }
            }
        }

        SDL_RenderClear(ren);
        if (bgTex) SDL_RenderCopy(ren, bgTex, nullptr, nullptr);

        for (auto& b : buttons) {
            SDL_Color c = b.isHovered ? SDL_Color{0,200,255,255} : b.color;
            SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
            SDL_RenderFillRect(ren, &b.rect);
            SDL_SetRenderDrawColor(ren, 255,255,255,255);
            SDL_RenderDrawRect(ren, &b.rect);
            renderText(ren, font, b.label, {255,255,255,255},
                       b.rect.x + 20, b.rect.y + 15);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    return next;
}
