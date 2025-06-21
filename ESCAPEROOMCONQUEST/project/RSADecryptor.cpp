#include "RSADecryptor.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <cmath>

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

void runRSADecyptor(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Surface* bgSurf = IMG_Load("assets/background.png");
    if (!bgSurf) {
        std::cerr << "Image load error: " << IMG_GetError() << "\n";
        return;
    }

    SDL_Texture* bgTex = SDL_CreateTextureFromSurface(renderer, bgSurf);
    SDL_FreeSurface(bgSurf);
    if (!bgTex) {
        std::cerr << "Texture creation error: " << SDL_GetError() << "\n";
        return;
    }

    std::string inputN, inputE, inputEnc, result;
    enum Focus { FOCUS_N, FOCUS_E, FOCUS_ENC } currentFocus = FOCUS_N;

    bool running = true;
    SDL_Event event;
    float animationTime = 0.0f;
    SDL_StartTextInput();

    SDL_Rect rectN = {200, 40, 500, 38};
    SDL_Rect rectE = {200, 100, 500, 38};
    SDL_Rect rectEnc = {200, 190, 500, 38};
    SDL_Rect decryptBtn = {50, 260, 120, 40};

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
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
                } else if (mx > rectN.x && mx < rectN.x + rectN.w && my > rectN.y && my < rectN.y + rectN.h) {
                    currentFocus = FOCUS_N;
                } else if (mx > rectE.x && mx < rectE.x + rectE.w && my > rectE.y && my < rectE.y + rectE.h) {
                    currentFocus = FOCUS_E;
                } else if (mx > rectEnc.x && mx < rectEnc.x + rectEnc.w && my > rectEnc.y && my < rectEnc.y + rectEnc.h) {
                    currentFocus = FOCUS_ENC;
                }
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

        SDL_Color labelColor = {255, 255, 255, 255};
        renderText(renderer, font, "Enter n:", labelColor, 50, 40);
        renderText(renderer, font, "Enter e:", labelColor, 50, 100);
        renderText(renderer, font, "Encrypted Text:", labelColor, 50, 160);
        renderText(renderer, font, "Result:", labelColor, 50, 320);

        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
        SDL_RenderDrawRect(renderer, &rectN);
        SDL_RenderDrawRect(renderer, &rectE);
        SDL_RenderDrawRect(renderer, &rectEnc);

        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 150);
        SDL_RenderDrawRect(renderer, currentFocus == FOCUS_N ? &rectN : currentFocus == FOCUS_E ? &rectE : &rectEnc);

        SDL_Color inputColor = {255, 255, 255, 255};
        renderText(renderer, font, inputN, inputColor, rectN.x + 10, rectN.y + 8);
        renderText(renderer, font, inputE, inputColor, rectE.x + 10, rectE.y + 8);
        renderText(renderer, font, inputEnc, inputColor, rectEnc.x + 10, rectEnc.y + 8);

        SDL_SetRenderDrawColor(renderer, 50, 200, 50, 255);
        SDL_RenderFillRect(renderer, &decryptBtn);
        renderText(renderer, font, "Decrypt", {30, 30, 30, 255}, decryptBtn.x + 10, decryptBtn.y + 5);

        SDL_Color resultColor = (result == "Access Denied. Try again." || result == "Invalid input") ? SDL_Color{255, 60, 60, 255} : SDL_Color{50, 255, 100, 255};
        renderText(renderer, font, result, resultColor, 50, 360);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_DestroyTexture(bgTex);
}
