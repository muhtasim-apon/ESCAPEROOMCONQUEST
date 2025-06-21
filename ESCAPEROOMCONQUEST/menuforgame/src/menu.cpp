#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

const int WINDOW_WIDTH = 768;
const int WINDOW_HEIGHT = 1152;

struct MenuButton {
    SDL_Rect rect;
    std::string label;
    SDL_Color color;
    bool hovered = false;
    bool clicked = false;
};

bool pointInRect(int x, int y, SDL_Rect& rect) {
    return (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h);
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                SDL_Color color, SDL_Rect& dstRect) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(texture, NULL, NULL, &dstRect.w, &dstRect.h);
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void showHighScores(SDL_Renderer* parentRenderer, TTF_Font* font) {
    SDL_Window* scoreWindow = SDL_CreateWindow("High Scores", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 400, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(scoreWindow, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event e;

    std::vector<std::pair<std::string, int>> scores;
    std::ifstream file("highscores.txt");
    std::string name;
    int score;
    while (file >> name >> score) {
        scores.emplace_back(name, score);
    }

    std::sort(scores.begin(), scores.end(), [](auto& a, auto& b) {
        return b.second < a.second;
    });

    bool open = true;
    while (open) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                open = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        int y = 50;
        if (scores.empty()) {
            SDL_Color color = {255, 255, 255};
            SDL_Rect r = {50, y, 0, 0};
            renderText(renderer, font, "No high scores yet!", color, r);
        } else {
            for (const auto& entry : scores) {
                std::string line = entry.first + ": " + std::to_string(entry.second);
                SDL_Color color = {255, 255, 255};
                SDL_Rect r = {50, y, 0, 0};
                renderText(renderer, font, line, color, r);
                y += 50;
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(scoreWindow);
}

void updateScore(const std::string& player, int points) {
    std::vector<std::pair<std::string, int>> scores;
    std::ifstream inFile("highscores.txt");
    std::string name;
    int score;
    bool found = false;
    while (inFile >> name >> score) {
        if (name == player) {
            score += points;
            found = true;
        }
        scores.emplace_back(name, score);
    }
    inFile.close();

    if (!found) {
        scores.emplace_back(player, points);
    }

    std::ofstream outFile("highscores.txt");
    for (const auto& pair : scores) {
        outFile << pair.first << " " << pair.second << "\n";
    }
}

std::string getPlayerName(SDL_Renderer* renderer, TTF_Font* font) {
    std::string nameInput;
    SDL_StartTextInput();
    SDL_Event e;
    bool entering = true;

    while (entering) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return "";
            else if (e.type == SDL_TEXTINPUT) {
                nameInput += e.text.text;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !nameInput.empty()) {
                    nameInput.pop_back();
                } else if (e.key.keysym.sym == SDLK_RETURN && !nameInput.empty()) {
                    entering = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_Color color = {255, 255, 255};
        SDL_Rect labelRect = {100, 200, 0, 0};
        renderText(renderer, font, "Enter your name:", color, labelRect);

        SDL_Rect inputRect = {100, 300, 0, 0};
        renderText(renderer, font, nameInput, color, inputRect);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    return nameInput;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() == -1 || IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "Failed to initialize SDL components\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Escape Room Conquest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* bgSurface = IMG_Load("menusection/menu_background.png");
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    TTF_Font* font = TTF_OpenFont("menusection/creepster.ttf", 36);
    TTF_Font* titleFont = TTF_OpenFont("menusection/creepster.ttf", 64);

    if (!font || !titleFont) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    std::vector<MenuButton> buttons = {
        {{270, 300, 0, 0}, "New Game",      {255, 255, 0}},
        {{270, 370, 0, 0}, "Resume Game",   {255, 255, 0}},
        {{270, 440, 0, 0}, "Help",          {255, 255, 0}},
        {{270, 510, 0, 0}, "Map",           {255, 255, 0}},
        {{270, 580, 0, 0}, "Highest Score", {255, 255, 0}},
        {{270, 650, 0, 0}, "Exit",          {255, 255, 0}}
    };

    bool running = true;
    SDL_Event e;

    while (running) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                for (size_t i = 0; i < buttons.size(); ++i) {
                    if (pointInRect(mouseX, mouseY, buttons[i].rect)) {
                        for (auto& b : buttons) b.clicked = false;
                        buttons[i].clicked = true;
                        if (i == 0) {
                            std::string playerName = getPlayerName(renderer, font);
                            if (!playerName.empty()) {
                                updateScore(playerName, 10);
                            }
                        }
                        else if (i == 1) std::cout << "Resume Game\n";
                        else if (i == 2) std::cout << "Help\n";
                        else if (i == 3) std::cout << "Map\n";
                        else if (i == 4) showHighScores(renderer, font);
                        else if (i == 5) running = false;
                    }
                }
            }
        }

        for (auto& btn : buttons) {
            btn.hovered = pointInRect(mouseX, mouseY, btn.rect);
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);

        SDL_Color titleColor = {255, 255, 255};
        SDL_Rect titleRect = {0, 100, 0, 0};
        renderText(renderer, titleFont, "Escape Room Conquest", titleColor, titleRect);
        titleRect.x = (WINDOW_WIDTH - titleRect.w) / 2;
        SDL_RenderCopy(renderer, SDL_GetRenderTarget(renderer), NULL, &titleRect);

        for (auto& btn : buttons) {
            SDL_Color textColor = btn.clicked ? SDL_Color{0, 0, 0} : (btn.hovered ? SDL_Color{255, 255, 255} : btn.color);
            SDL_Rect textRect = btn.rect;
            renderText(renderer, font, btn.label, textColor, textRect);
            btn.rect = textRect;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    TTF_CloseFont(titleFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
