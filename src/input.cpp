#include "input.h"
#include "Utils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

bool getPlayerName(SDL_Renderer* ren, std::string& outName) {
    SDL_StartTextInput();
    outName.clear();
    SDL_Event e;
    TTF_Font* font = TTF_OpenFont("assets/arial.ttf", 28);
    if (!font) return false;
    SDL_Color white{255,255,255,255};

    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_StopTextInput();
                TTF_CloseFont(font);
                return false;
            }
            if (e.type == SDL_TEXTINPUT) {
                outName += e.text.text;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !outName.empty())
                    outName.pop_back();
                else if (e.key.keysym.sym == SDLK_RETURN && !outName.empty()) {
                    SDL_StopTextInput();
                    TTF_CloseFont(font);
                    return true;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        renderText(ren, font, "Enter Your Name:", white, 200, 200);
        renderText(ren, font, outName + "_", white, 200, 250);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
}
