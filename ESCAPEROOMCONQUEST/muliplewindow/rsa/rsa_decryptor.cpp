#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

// Modular exponentiation
long long mod_exp(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % mod;
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

std::string decryptRSA(const std::string& encryptedStr, long long d, long long n) {
    std::stringstream ss(encryptedStr);
    std::string token, result;
    while (ss >> token) {
        long long cipher = std::stoll(token);
        char decryptedChar = static_cast<char>(mod_exp(cipher, d, n));
        result += decryptedChar;
    }
    return result;
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// RSA GUI logic wrapped in a function
int launchRSAGUI(const std::string& playerName) {
    SDL_Window* window = SDL_CreateWindow("RSA GUI Decryptor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 24);
    SDL_Surface* bgSurf = IMG_Load("background.png");
    SDL_Texture* bgTex = SDL_CreateTextureFromSurface(renderer, bgSurf);
    SDL_FreeSurface(bgSurf);

    std::string inputN, inputE, inputEnc, result;
    enum Focus { FOCUS_N, FOCUS_E, FOCUS_ENC } currentFocus = FOCUS_N;

    bool running = true;
    SDL_Event event;
    float animationTime = 0.0f;
    SDL_StartTextInput();

    const int horiz_padding = 14;
    const int vert_padding = 8;

    SDL_Rect rectN   = {200, 40, 500, 38};
    SDL_Rect rectE   = {200, 100, 500, 38};
    SDL_Rect rectEnc = {200, 190, 500, 38};
    SDL_Rect decryptBtn = {50, 260, 120, 40};

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x, my = event.button.y;
                if (mx > decryptBtn.x && mx < decryptBtn.x + decryptBtn.w && my > decryptBtn.y && my < decryptBtn.y + decryptBtn.h) {
                    try {
                        long long n = std::stoll(inputN);
                        long long e = std::stoll(inputE);
                        if (n == 2537 && e == 13 && inputEnc == "2081 2182 2024") {
                            result = "Curzon is haunted";
                        } else {
                            result = "Access Denied. Try again.";
                        }
                    } catch (...) {
                        result = "Invalid input";
                    }
                } else if (mx > rectN.x && mx < rectN.x + rectN.w && my > rectN.y && my < rectN.y + rectN.h) currentFocus = FOCUS_N;
                else if (mx > rectE.x && mx < rectE.x + rectE.w && my > rectE.y && my < rectE.y + rectE.h) currentFocus = FOCUS_E;
                else if (mx > rectEnc.x && mx < rectEnc.x + rectEnc.w && my > rectEnc.y && my < rectEnc.y + rectEnc.h) currentFocus = FOCUS_ENC;
            } else if (event.type == SDL_TEXTINPUT) {
                if (currentFocus == FOCUS_N) inputN += event.text.text;
                else if (currentFocus == FOCUS_E) inputE += event.text.text;
                else if (currentFocus == FOCUS_ENC) inputEnc += event.text.text;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE) {
                if (currentFocus == FOCUS_N && !inputN.empty()) inputN.pop_back();
                else if (currentFocus == FOCUS_E && !inputE.empty()) inputE.pop_back();
                else if (currentFocus == FOCUS_ENC && !inputEnc.empty()) inputEnc.pop_back();
            }
        }

        animationTime += 0.05f;
        int bgOffsetY = static_cast<int>(std::sin(animationTime) * 5.0);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        SDL_Rect bgDst = {0, bgOffsetY, 900, 600};
        SDL_RenderCopy(renderer, bgTex, nullptr, &bgDst);

        SDL_Color labelColor = {255,255,255,255};
        renderText(renderer, font, "Enter n:", labelColor, 50, 40);
        renderText(renderer, font, "Enter e:", labelColor, 50, 100);
        renderText(renderer, font, "Encrypted Text:", labelColor, 50, 160);
        renderText(renderer, font, "Result:", labelColor, 50, 320);

        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
        SDL_RenderDrawRect(renderer, &rectN); SDL_RenderDrawRect(renderer, &rectE); SDL_RenderDrawRect(renderer, &rectEnc);
        SDL_Rect h; h = (currentFocus == FOCUS_N) ? rectN : (currentFocus == FOCUS_E) ? rectE : rectEnc;
        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 150);
        SDL_RenderDrawRect(renderer, &h);

        SDL_Color inputColor = {255,255,255,255};
        renderText(renderer, font, inputN, inputColor, rectN.x + horiz_padding, rectN.y + vert_padding);
        renderText(renderer, font, inputE, inputColor, rectE.x + horiz_padding, rectE.y + vert_padding);
        renderText(renderer, font, inputEnc, inputColor, rectEnc.x + horiz_padding, rectEnc.y + vert_padding);

        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        SDL_RenderFillRect(renderer, &decryptBtn);
        renderText(renderer, font, "Decrypt", {30,30,30,255}, decryptBtn.x + 20, decryptBtn.y + 7);

        SDL_Color resultColor = (result == "Access Denied. Try again." || result == "Invalid input") ? SDL_Color{255,60,60,255} : SDL_Color{50,255,100,255};
        renderText(renderer, font, result, resultColor, 50, 360);

        renderText(renderer, font, "Welcome, " + playerName + "!", {255, 255, 100, 255}, 600, 10);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(bgTex);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}

// Initial player name entry screen
int main() {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0 || !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "Init error: " << SDL_GetError() << std::endl; return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Enter Player Name", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 28);
    SDL_StartTextInput();

    std::string playerName;
    SDL_Event event;
    bool entering = true;
    while (entering) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return 0;
            else if (event.type == SDL_TEXTINPUT) playerName += event.text.text;
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) playerName.pop_back();
                else if (event.key.keysym.sym == SDLK_RETURN && !playerName.empty()) entering = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);
        renderText(renderer, font, "Enter your name:", {255, 255, 255, 255}, 320, 200);
        renderText(renderer, font, playerName, {255, 255, 255, 255}, 320, 260);
        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);

    return launchRSAGUI(playerName);
}
