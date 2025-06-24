#include "MapModule.h"
#include "GameState.h"
#include "GameContext.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <thread>

// --- world setup ---
static const int SCREEN_W = 1024, SCREEN_H = 768;
static int WORLD_W = 1600, WORLD_H = 1200;
static SDL_Texture* bgTex    = nullptr;
static SDL_Texture* playerTx = nullptr;
static SDL_Rect     player{50,100,64,64};
static SDL_Rect     camera{0,0,SCREEN_W,SCREEN_H};
static SDL_Rect     obs1{200,80,100,80};
static SDL_Rect     obs2{450,80,100,80};
static SDL_Rect     obs3{720,80,100,80};

static bool initMedia(SDL_Renderer* ren) {
    if(bgTex && playerTx) return true;
    bgTex    = IMG_LoadTexture(ren,"assets/background.png");
    playerTx = IMG_LoadTexture(ren,"assets/player.png");
    if (bgTex) SDL_QueryTexture(bgTex,nullptr,nullptr,&WORLD_W,&WORLD_H);
    return bgTex && playerTx;
}

static bool canMove(int dx,int dy) {
    SDL_Rect tmp=player; tmp.x+=dx; tmp.y+=dy;
    return !SDL_HasIntersection(&tmp,&obs1)
        && !SDL_HasIntersection(&tmp,&obs2)
        && !SDL_HasIntersection(&tmp,&obs3);
}

static void updateCamera() {
    camera.x = player.x + player.w/2 - camera.w/2;
    camera.y = player.y + player.h/2 - camera.h/2;
    camera.x = std::max(0, std::min(camera.x, WORLD_W - camera.w));
    camera.y = std::max(0, std::min(camera.y, WORLD_H - camera.h));
}

static void renderFrame(SDL_Renderer* ren) {
    SDL_RenderClear(ren);
    SDL_Rect dst={0,0,SCREEN_W,SCREEN_H};
    SDL_RenderCopy(ren,bgTex,&camera,&dst);
    SDL_Rect onS={player.x-camera.x, player.y-camera.y,player.w,player.h};
    SDL_RenderCopy(ren,playerTx,nullptr,&onS);
    SDL_RenderPresent(ren);
}

GameState MapModule::run(GameContext& ctx) {
    auto ren=ctx.renderer;
    if(!initMedia(ren)) return GameState::EXIT;
    bool quit=false; SDL_Event e;
    while(!quit){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) quit=true;
            else if(e.type==SDL_KEYDOWN){
                switch(e.key.keysym.sym){
                  case SDLK_LEFT:  if(canMove(-10,0)) player.x-=10; break;
                  case SDLK_RIGHT: if(canMove(10,0)) player.x+=10; break;
                  case SDLK_UP:    if(canMove(0,-10)) player.y-=10; break;
                  case SDLK_DOWN:  if(canMove(0,10)) player.y+=10; break;
                }
            }
            else if(e.type==SDL_MOUSEBUTTONDOWN){
                int mx,my; SDL_GetMouseState(&mx,&my);
                SDL_Rect click{mx+camera.x,my+camera.y,1,1};
                if(SDL_HasIntersection(&click,&obs1)) {/* launch puzzle 1 */}
                if(SDL_HasIntersection(&click,&obs2)) {/* launch puzzle 2 */}
            }
        }
        updateCamera();
        renderFrame(ren);
        SDL_Delay(16);
    }
    return GameState::EXIT;
}
