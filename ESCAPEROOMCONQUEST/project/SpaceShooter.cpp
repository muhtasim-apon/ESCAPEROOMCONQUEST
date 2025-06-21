#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>  // ✅ ADD THIS
#include "SpaceShooter.h"
#include "Utils.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <algorithm>


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int WIN_SCORE = 100;

struct Bullet {
    SDL_Rect rect;
    int speed = -10;
};

struct Enemy {
    SDL_Rect rect;
    std::string label;
    int speed = 1;
};

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return SDL_HasIntersection(&a, &b);
}

std::string getPlayerName(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_StartTextInput();
    std::string name;
    SDL_Event e;
    bool done = false;
    SDL_Color white = {255, 255, 255};

    while (!done) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderText(renderer, font, "Enter Your Name:", white, 250, 200);
        renderText(renderer, font, name + "_", white, 250, 250);
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return "Player";
            if (e.type == SDL_TEXTINPUT) name += e.text.text;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !name.empty()) name.pop_back();
                if (e.key.keysym.sym == SDLK_RETURN && !name.empty()) done = true;
            }
        }
    }

    SDL_StopTextInput();
    return name;
}

std::string generateEncryptedCode() {
    std::string code = "Encrypted code: ";
    for (int i = 0; i < 16; ++i) {
        code += 'A' + rand() % 26;
    }
    return code;
}

void showEndScreen(SDL_Renderer* renderer, TTF_Font* font, const std::string& name, int score, bool won) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderText(renderer, font, "Game Over!", {255, 255, 255}, 320, 180);
    renderText(renderer, font, "Player: " + name, {255, 255, 255}, 300, 230);
    renderText(renderer, font, "Score: " + std::to_string(score), {255, 255, 255}, 300, 270);

    if (won) {
        renderText(renderer, font, generateEncryptedCode(), {0, 255, 0}, 220, 310);
    } else {
        renderText(renderer, font, "Try Again!", {255, 0, 0}, 300, 310);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(5000);
}

void runSpaceShooter(SDL_Renderer* renderer, TTF_Font* font) {
    srand(static_cast<unsigned>(time(NULL)));

    SDL_Surface* bgSurface = IMG_Load("assets/space_background.png");
    SDL_Surface* ship1Surface = IMG_Load("assets/ship1.png");
    SDL_Surface* ship2Surface = IMG_Load("assets/ship2.png");

    if (!bgSurface || !ship1Surface || !ship2Surface) {
        std::cerr << "Image load error: " << IMG_GetError() << "\n";
        if (bgSurface) SDL_FreeSurface(bgSurface);
        if (ship1Surface) SDL_FreeSurface(ship1Surface);
        if (ship2Surface) SDL_FreeSurface(ship2Surface);
        return;
    }

    SDL_Texture* bgTex = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_Texture* playerTex = SDL_CreateTextureFromSurface(renderer, ship1Surface);
    SDL_Texture* enemyTex = SDL_CreateTextureFromSurface(renderer, ship2Surface);

    SDL_FreeSurface(bgSurface);
    SDL_FreeSurface(ship1Surface);
    SDL_FreeSurface(ship2Surface);

    std::string playerName = getPlayerName(renderer, font);
    SDL_Rect player = {SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT - 60, 50, 40};
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::string labels[] = {"PROJECT", "QUIZ", "LAB", "EXAM"};

    int score = 0;
    bool quit = false;
    SDL_Event e;
    Uint32 lastSpawnTime = SDL_GetTicks();

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                Bullet b;
                b.rect = {player.x + player.w / 2 - 5, player.y, 10, 20};
                bullets.push_back(b);
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT] && player.x > 0) player.x -= 7;
        if (keys[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - player.w) player.x += 7;

        for (auto& b : bullets) b.rect.y += b.speed;
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b) {
            return b.rect.y < 0;
        }), bullets.end());

        Uint32 current = SDL_GetTicks();
        if (current - lastSpawnTime > 1000) {
            Enemy en;
            en.rect = {rand() % (SCREEN_WIDTH - 60), 0, 60, 40};
            en.label = labels[rand() % 4];
            en.speed = 2 + rand() % 3;
            enemies.push_back(en);
            lastSpawnTime = current;
        }

        for (auto& en : enemies) en.rect.y += en.speed;

        for (size_t i = 0; i < bullets.size(); ++i) {
            bool hit = false;
            for (size_t j = 0; j < enemies.size(); ++j) {
                if (checkCollision(bullets[i].rect, enemies[j].rect)) {
                    bullets.erase(bullets.begin() + i);
                    enemies.erase(enemies.begin() + j);
                    score += 10;
                    --i;
                    hit = true;
                    break;
                }
            }
            if (hit) break;
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [&](Enemy& en) {
            if (en.rect.y > SCREEN_HEIGHT) {
                quit = true;
                return true;
            }
            return false;
        }), enemies.end());

        if (score >= WIN_SCORE) quit = true;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTex, NULL, NULL);
        SDL_RenderCopy(renderer, playerTex, NULL, &player);

        int glow = 128 + 127 * sin(SDL_GetTicks() / 300.0);
        SDL_Color glowColor = {(Uint8)glow, (Uint8)glow, (Uint8)glow, 255};

        for (auto& en : enemies) {
            SDL_RenderCopy(renderer, enemyTex, NULL, &en.rect);
            renderText(renderer, font, en.label, glowColor, en.rect.x + 5, en.rect.y + 10);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        for (auto& b : bullets) {
            SDL_RenderFillRect(renderer, &b.rect);
        }

        renderText(renderer, font, "Score: " + std::to_string(score), {255, 255, 255}, 10, 10);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    showEndScreen(renderer, font, playerName, score, score >= WIN_SCORE);
    SDL_DestroyTexture(bgTex);
    SDL_DestroyTexture(playerTex);
    SDL_DestroyTexture(enemyTex);
}
