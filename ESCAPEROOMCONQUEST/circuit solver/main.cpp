#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

const int WIN_W = 800, WIN_H = 600;
const int COMP_COUNT = 6;
const SDL_Point targetSlots[COMP_COUNT] = {
    {124, 457}, {530, 178}, {534, 282}, {433,494 }, {536, 452}, {125, 305}, 
};
const char* fileNames[COMP_COUNT] = {
    "battery.png",
    "resistor.png",
    "capacitor.png",
    "diode.png",
    "voltmeter.png",
    "ammeter.png"
};

bool isNear(int x1, int y1, int x2, int y2, int range = 40) {
    return (std::abs(x1 - x2) < range && std::abs(y1 - y2) < range);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Circuit Puzzle Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Renderer* ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    SDL_Texture* background = IMG_LoadTexture(ren, "background.png");
    SDL_Texture* ledTex = IMG_LoadTexture(ren, "led.png");
    SDL_Texture* compTex[COMP_COUNT];
    for (int i = 0; i < COMP_COUNT; ++i) {
        compTex[i] = IMG_LoadTexture(ren, fileNames[i]);
        if (!compTex[i]) {
            std::cerr << "IMG_LoadTexture Error for " << fileNames[i] << ": " << IMG_GetError() << std::endl;
        }
    }

    struct Comp { SDL_Rect rect; bool placed; };
    std::vector<Comp> comps(COMP_COUNT);
    for (int i = 0; i < COMP_COUNT; ++i) {
        comps[i].rect = {100 + i * 100, 400, 64, 64};
        comps[i].placed = false;
    }

    bool quit = false;
    bool paused = false, solved = false;
    bool dragging = false;
    int dragged = -1;
    int offsetX = 0, offsetY = 0;
    Uint32 startTicks = SDL_GetTicks();
    Uint32 pausedTicks = 0;
    const int TIME_LIMIT = 60;

    std::string unlockKey;
    bool inputActive = false;
    std::string userInput;
    std::string accessMsg;

    SDL_Color white = {255,255,255,255};
    SDL_Color green = {0,255,0,255};
    SDL_Color red   = {255,0,0,255};
    SDL_Color black = {0,0,0,255};

    std::srand((unsigned)std::time(nullptr));
    SDL_StartTextInput();
    inputActive = false;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE && !solved) {
                    paused = !paused;
                    if (paused) {
                        pausedTicks = SDL_GetTicks() - startTicks;
                    } else {
                        startTicks = SDL_GetTicks() - pausedTicks;
                    }
                }
                if (solved && inputActive) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !userInput.empty()) {
                        userInput.pop_back();
                    }
                    if ((e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)) {
                        std::string code = unlockKey.substr(unlockKey.find(':') + 2);
                        if (userInput == code) accessMsg = "Access Granted!";
                        else                   accessMsg = "Wrong Key!";
                    }
                }
            }
            if (!paused && !solved) {
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x, my = e.button.y;
                    for (int i = 0; i < COMP_COUNT; ++i) {
                        SDL_Rect r = comps[i].rect;
                        if (mx > r.x && mx < r.x + r.w && my > r.y && my < r.y + r.h) {
                            dragging = true;
                            dragged = i;
                            offsetX = mx - r.x;
                            offsetY = my - r.y;
                            break;
                        }
                    }
                }
                if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                    dragging = false;
                    dragged = -1;
                }
                if (e.type == SDL_MOUSEMOTION && dragging && dragged != -1) {
                    comps[dragged].rect.x = e.motion.x - offsetX;
                    comps[dragged].rect.y = e.motion.y - offsetY;
                }
            }
            if (e.type == SDL_TEXTINPUT && solved && inputActive) {
                userInput += e.text.text;
            }
        }

        Uint32 now = SDL_GetTicks();
        int secLeft;

        if (paused || solved) {
            secLeft = TIME_LIMIT - (int)(pausedTicks / 1000);
        } else {
            pausedTicks = now - startTicks;
            secLeft = TIME_LIMIT - (int)(pausedTicks / 1000);
        }

        if (!paused && !solved && secLeft <= 0) {
            solved = true;
            unlockKey = "Time's up! Try again.";
            inputActive = false;
        }

        if (!solved) {
            bool all = true;
            for (int i = 0; i < COMP_COUNT; ++i) {
                if (isNear(comps[i].rect.x, comps[i].rect.y, targetSlots[i].x, targetSlots[i].y))
                    comps[i].placed = true;
                else {
                    comps[i].placed = false;
                    all = false;
                }
            }
            if (all) {
                solved = true;
                int keyNum = 1000 + std::rand() % 9000;
                unlockKey = "Puzzle Solved! Unlock Key: " + std::to_string(keyNum);
                inputActive = true;
            }
        }

        SDL_SetRenderDrawColor(ren, 20,20,20,255);
        SDL_RenderClear(ren);

        if (background) {
            SDL_Rect bgRect = {0, 0, WIN_W, WIN_H};
            SDL_RenderCopy(ren, background, NULL, &bgRect);
        }

        if (ledTex) {
            SDL_Rect lr = {700,20,60,60};
            if (solved && inputActive)
                SDL_SetTextureColorMod(ledTex, 0,255,0);
            else
                SDL_SetTextureColorMod(ledTex, 100,100,100);
            SDL_RenderCopy(ren, ledTex, NULL, &lr);
        }

        for (int i = 0; i < COMP_COUNT; ++i) {
            SDL_SetRenderDrawColor(ren, 100,255,200,120);
            SDL_Rect sr = {targetSlots[i].x, targetSlots[i].y, 48, 48};
            SDL_RenderDrawRect(ren, &sr);
        }

        for (int i = 0; i < COMP_COUNT; ++i) {
            if (compTex[i]) SDL_RenderCopy(ren, compTex[i], NULL, &comps[i].rect);
        }

        {
            std::stringstream ss;
            ss << "Time Left: " << (secLeft>0?secLeft:0) << "s";
            SDL_Surface* ts = TTF_RenderText_Blended(font, ss.str().c_str(), black);
            if (ts) {
                SDL_Texture* tt = SDL_CreateTextureFromSurface(ren, ts);
                SDL_Rect tr = {10,10,ts->w,ts->h}; SDL_RenderCopy(ren, tt, NULL, &tr);
                SDL_FreeSurface(ts); SDL_DestroyTexture(tt);
            }
        }

        if (solved) {
            SDL_Surface* rs = TTF_RenderText_Blended(font, unlockKey.c_str(), (inputActive? green:white));
            if (rs) {
                SDL_Texture* rt = SDL_CreateTextureFromSurface(ren, rs);
                SDL_Rect rr = {150,500,rs->w,rs->h}; SDL_RenderCopy(ren, rt, NULL, &rr);
                SDL_FreeSurface(rs); SDL_DestroyTexture(rt);
            }
            if (inputActive) {
                SDL_Rect box = {250,550,300,36};
                SDL_SetRenderDrawColor(ren, 255,255,255,60); SDL_RenderFillRect(ren,&box);
                SDL_SetRenderDrawColor(ren, 100,255,200,180); SDL_RenderDrawRect(ren,&box);
                SDL_Surface* ps = TTF_RenderText_Blended(font, "Enter Key:", white);
                if (ps) {
                    SDL_Texture* pt = SDL_CreateTextureFromSurface(ren, ps);
                    SDL_Rect pr = {box.x-ps->w-10,box.y+4,ps->w,ps->h};
                    SDL_RenderCopy(ren,pt,NULL,&pr);
                    SDL_FreeSurface(ps); SDL_DestroyTexture(pt);
                }
                SDL_Surface* us = TTF_RenderText_Blended(font, userInput.c_str(), white);
                if (us) {
                    SDL_Texture* ut = SDL_CreateTextureFromSurface(ren, us);
                    SDL_Rect ur = {box.x+10,box.y+4,us->w,us->h};
                    SDL_RenderCopy(ren,ut,NULL,&ur);
                    SDL_FreeSurface(us); SDL_DestroyTexture(ut);
                }
                if (!accessMsg.empty()) {
                    SDL_Color col = (accessMsg=="Access Granted!"?green:red);
                    SDL_Surface* am = TTF_RenderText_Blended(font,accessMsg.c_str(),col);
                    if (am) {
                        SDL_Texture* at = SDL_CreateTextureFromSurface(ren,am);
                        SDL_Rect ar = {box.x+50,box.y-40,am->w,am->h};
                        SDL_RenderCopy(ren,at,NULL,&ar);
                        SDL_FreeSurface(am); SDL_DestroyTexture(at);
                    }
                }
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    for (int i=0; i<COMP_COUNT; ++i) if(compTex[i]) SDL_DestroyTexture(compTex[i]);
    if (ledTex) SDL_DestroyTexture(ledTex);
    if (background) SDL_DestroyTexture(background);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
