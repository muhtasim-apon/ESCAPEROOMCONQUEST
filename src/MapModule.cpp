#include "MapModule.h"
#include "Utils.h"          // for any helper like loadTexture()
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

// screen dimensions
static const int SCREEN_W = 800;
static const int SCREEN_H = 600;

// world size (must match your background)
static const int WORLD_W = 1600;
static const int WORLD_H = 1200;

// camera & player
static SDL_Rect camera = { 0, 0, SCREEN_W, SCREEN_H };
static SDL_Rect player = { 100, 100, 64, 64 };

// background & player textures
static SDL_Texture* bgTex     = nullptr;
static SDL_Texture* playerTex = nullptr;

// initialize assets
static bool loadAssets(SDL_Renderer* ren) {
    if (!bgTex) {
        bgTex = IMG_LoadTexture(ren, "assets/firstfloor_background.png");
        if (!bgTex) { std::cerr<<"bg load failed: "<<IMG_GetError()<<"\n"; return false; }
    }
    if (!playerTex) {
        playerTex = IMG_LoadTexture(ren, "assets/player.png");
        if (!playerTex) { std::cerr<<"player load failed: "<<IMG_GetError()<<"\n"; return false; }
    }
    return true;
}

// cleanup
static void unloadAssets() {
    if (bgTex)     SDL_DestroyTexture(bgTex);
    if (playerTex) SDL_DestroyTexture(playerTex);
}

// simple input
static void handleInput(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    switch (e.key.keysym.sym) {
    case SDLK_LEFT:  player.x -= 10; break;
    case SDLK_RIGHT: player.x += 10; break;
    case SDLK_UP:    player.y -= 10; break;
    case SDLK_DOWN:  player.y += 10; break;
    }
    // clamp player to world
    if (player.x < 0)             player.x = 0;
    if (player.y < 0)             player.y = 0;
    if (player.x > WORLD_W - player.w) player.x = WORLD_W - player.w;
    if (player.y > WORLD_H - player.h) player.y = WORLD_H - player.h;
}

// update camera so player is centered (with an offset if you like)
static void updateCamera() {
    camera.x = player.x + player.w/2 - camera.w/2;
    camera.y = player.y + player.h/2 - camera.h/2;
    // clamp
    if (camera.x < 0)               camera.x = 0;
    if (camera.y < 0)               camera.y = 0;
    if (camera.x > WORLD_W - camera.w) camera.x = WORLD_W - camera.w;
    if (camera.y > WORLD_H - camera.h) camera.y = WORLD_H - camera.h;
}

// render the scene through the camera
static void renderScene(SDL_Renderer* ren) {
    SDL_RenderClear(ren);

    // draw background
    SDL_Rect src = camera;
    SDL_Rect dst = { 0, 0, SCREEN_W, SCREEN_H };
    SDL_RenderCopy(ren, bgTex, &src, &dst);

    // draw player relative to camera
    SDL_Rect onScreen = {
        player.x - camera.x,
        player.y - camera.y,
        player.w,
        player.h
    };
    SDL_RenderCopy(ren, playerTex, nullptr, &onScreen);

    SDL_RenderPresent(ren);
}

GameState MapModule::run(GameContext& ctx) {
    SDL_Renderer* ren = ctx.renderer;
    if (!loadAssets(ren)) return GameState::EXIT;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            handleInput(e);
        }

        updateCamera();
        renderScene(ren);

        SDL_Delay(16);
    }

    unloadAssets();
    // After map demo, you can transition to next state:
    return GameState::PUZZLE;  // or whichever state should follow MAP
}
