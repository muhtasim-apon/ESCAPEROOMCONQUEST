// src/Leaderboard.cpp
#include "Leaderboard.h"
#include "Utils.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>

namespace Leaderboard {
    static std::vector<LeaderboardEntry> entries;

    void loadFromFile(const std::string& filename) {
        entries.clear();
        std::ifstream in(filename);
        std::string line;
        while (std::getline(in,line)) {
            std::istringstream ss(line);
            std::string n,t;
            if (std::getline(ss,n,',') && std::getline(ss,t)) {
                entries.push_back({n, std::stof(t)});
            }
        }
    }

    GameState run(SDL_Renderer* ren, TTF_Font* font) {
        SDL_Texture* bg = loadTexture(ren, "assets/leaderboard.png");
        SDL_Rect backBtn{400,600-80,200,50};
        bool loop = true; SDL_Event e;

        while (loop) {
            while (SDL_PollEvent(&e)) {
                if (e.type==SDL_QUIT) return GameState::EXIT;
                if (e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE)
                    loop=false;
                if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {
                    int x=e.button.x,y=e.button.y;
                    if (x>=backBtn.x&&x<=backBtn.x+backBtn.w
                     &&y>=backBtn.y&&y<=backBtn.y+backBtn.h)
                        loop=false;
                }
            }
            SDL_RenderClear(ren);
            if (bg) SDL_RenderCopy(ren,bg,nullptr,nullptr);

            int y0=200;
            for (size_t i=0;i<entries.size();++i) {
                char buf[80];
                snprintf(buf,sizeof(buf),"%2zu. %-10s  %.2f",
                         i+1, entries[i].name.c_str(), entries[i].time);
                renderText(ren,font,buf,{255,255,255,255},300,y0+int(i)*40);
            }

            // back button
            SDL_SetRenderDrawColor(ren,200,50,50,200);
            SDL_RenderFillRect(ren,&backBtn);
            SDL_SetRenderDrawColor(ren,255,255,255,255);
            SDL_RenderDrawRect(ren,&backBtn);
            renderText(ren,font,"Back",{255,255,255,255},
                       backBtn.x+60, backBtn.y+10);

            SDL_RenderPresent(ren);
            SDL_Delay(16);
        }

        if (bg) SDL_DestroyTexture(bg);
        return GameState::MAP;
    }
}
