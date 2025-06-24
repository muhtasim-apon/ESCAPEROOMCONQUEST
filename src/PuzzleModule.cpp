#include "PuzzleModule.h"
#include "Utils.h"            // for renderText, loadTexture
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>
#include <iostream>

// --- constants & types ---
static constexpr int SCREEN_W = 1024;
static constexpr int SCREEN_H = 768;
static constexpr int PUZZLE_TIME_LIMIT = 30;

struct Puzzle {
    std::string question;
    std::string answer;
};

// Wrap-and-render convenience (returns texture, outRect set)
static SDL_Texture* renderWrapped(SDL_Renderer* ren, TTF_Font* f,
    const std::string& text, SDL_Color col, SDL_Rect& out, int wrapW=800)
{
    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(f, text.c_str(), col, wrapW);
    if (!surf) return nullptr;
    SDL_Texture* tx = SDL_CreateTextureFromSurface(ren, surf);
    out = {0,0, surf->w, surf->h};
    SDL_FreeSurface(surf);
    return tx;
}

GameState PuzzleModule::run(GameContext& ctx) {
    auto ren = ctx.renderer;

    // load assets once
    static TTF_Font* font = nullptr;
    static SDL_Texture* bgTex = nullptr;
    static Mix_Music*    bgm = nullptr;
    static Mix_Chunk*    sfxCorrect = nullptr;
    static Mix_Chunk*    sfxWrong   = nullptr;
    if (!font) {
        font = TTF_OpenFont("assets/arial.ttf", 24);
        bgTex = loadTexture(ren,"assets/puzzleimage.png");
        bgm  = Mix_LoadMUS("assets/puzzleGame.wav");
        sfxCorrect = Mix_LoadWAV("assets/correct.wav");
        sfxWrong   = Mix_LoadWAV("assets/wrong.wav");
        Mix_PlayMusic(bgm, -1);
    }
    if (!font||!bgTex) {
        SDL_Log("Puzzle assets failed\n");
        return GameState::EXIT;
    }

    // prepare puzzles
    std::vector<Puzzle> puzzles = {
        {"I have keys but no locks, I have space but no room. What am I?","keyboard"},
        {"What has to be broken before you use it?","egg"},
        {"The more you take, the more you leave behind. What am I?","footsteps"},
    };

    // --- name entry screen ---
    std::string playerName;
    SDL_StartTextInput();
    SDL_Event e;
    SDL_Color white{255,255,255,255};
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) { SDL_StopTextInput(); return GameState::EXIT; }
            if (e.type==SDL_TEXTINPUT) playerName += e.text.text;
            if (e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_RETURN && !playerName.empty())
                goto afterName;
            if (e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_BACKSPACE && !playerName.empty())
                playerName.pop_back();
        }
        SDL_SetRenderDrawColor(ren,0,0,0,255);
        SDL_RenderClear(ren);
        SDL_Rect r;
        auto t1 = renderWrapped(ren,font,"Enter your name:",white,r);
        r.x=(SCREEN_W-r.w)/2; r.y=200; SDL_RenderCopy(ren,t1,nullptr,&r); SDL_DestroyTexture(t1);
        auto t2 = renderWrapped(ren,font,playerName+"_",white,r);
        r.x=(SCREEN_W-r.w)/2; r.y=260; SDL_RenderCopy(ren,t2,nullptr,&r); SDL_DestroyTexture(t2);
        SDL_RenderPresent(ren);
    }
afterName:
    SDL_StopTextInput();

    // main puzzle loop
    int  idx = 0;
    bool running=true, started=false, solved=false, failed=false;
    Uint32 startTicks=0;
    std::string answer;

    // clickable “monitor” area:
    SDL_Rect area{(SCREEN_W-512)/2,256,512,320};

    while (running) {
        // manage text input state
        if (started && !SDL_IsTextInputActive()) SDL_StartTextInput();
        if ((!started||solved||failed) && SDL_IsTextInputActive()) SDL_StopTextInput();

        // events
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) return GameState::EXIT;
            // start puzzle on click
            if (!started && e.type==SDL_MOUSEBUTTONDOWN) {
                int mx,my; SDL_GetMouseState(&mx,&my);
                if (mx>=area.x&&mx<area.x+area.w && my>=area.y&&my<area.y+area.h) {
                    started=true; solved=failed=false; answer.clear();
                    startTicks=SDL_GetTicks();
                }
            }
            // typing answer
            if (started && !solved && !failed) {
                if (e.type==SDL_TEXTINPUT) answer += e.text.text;
                if (e.type==SDL_KEYDOWN) {
                    if (e.key.keysym.sym==SDLK_BACKSPACE && !answer.empty())
                        answer.pop_back();
                    else if (e.key.keysym.sym==SDLK_RETURN) {
                        if (answer==puzzles[idx].answer) {
                            Mix_PlayChannel(-1,sfxCorrect,0);
                            solved=true;
                        } else {
                            Mix_PlayChannel(-1,sfxWrong,0);
                            failed=true;
                        }
                    }
                }
            }
            // advance on SPACE
            if ((solved||failed) && e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_SPACE) {
                idx++;
                if (idx < (int)puzzles.size()) {
                    started=true; solved=failed=false; answer.clear();
                    startTicks=SDL_GetTicks();
                } else {
                    running=false;
                }
            }
        }

        // timeout
        if (started && !solved && !failed) {
            int left = PUZZLE_TIME_LIMIT - int((SDL_GetTicks()-startTicks)/1000);
            if (left<=0) { Mix_PlayChannel(-1,sfxWrong,0); failed=true; }
        }

        // draw
        SDL_SetRenderDrawColor(ren,0,0,0,255);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren,bgTex,nullptr,nullptr);

        SDL_Rect r;
        SDL_Texture* txt = nullptr;

        if (!started) {
            txt = renderWrapped(ren,font,
                "Click the screen to start the puzzle",
                white,r);
        }
        else if (solved) {
            txt = renderWrapped(ren,font,
                "Correct! Press SPACE for next",white,r);
        }
        else if (failed) {
            txt = renderWrapped(ren,font,
                "Wrong! Press SPACE to retry",white,r);
        }
        else {
            int left = PUZZLE_TIME_LIMIT - int((SDL_GetTicks()-startTicks)/1000);
            std::string full = puzzles[idx].question
                + "\n\nYour Answer: " + answer
                + "\n\nTime Left: " + std::to_string(left);
            txt = renderWrapped(ren,font, full, white,r);
        }

        if (txt) {
            r.x = (SCREEN_W-r.w)/2;
            r.y = SCREEN_H - r.h - 20;
            SDL_RenderCopy(ren,txt,nullptr,&r);
            SDL_DestroyTexture(txt);
        }

        // welcome banner
        auto wtxt = renderWrapped(ren,font,"Player: "+playerName,white,r);
        r.x=20; r.y=20;
        SDL_RenderCopy(ren,wtxt,nullptr,&r);
        SDL_DestroyTexture(wtxt);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    // done → next state
    return GameState::MAP;
}
