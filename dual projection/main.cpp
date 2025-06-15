#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

const int WIDTH = 800;
const int HEIGHT = 600;
const int CELL = 40;
const int MARGIN = 60;
const int COLS = 13;
const int ROWS = 11;
const int GRID_ORIGIN_X = MARGIN;
const int GRID_ORIGIN_Y = HEIGHT - MARGIN;

struct Vec2 {
    double x, y;
    Vec2 operator+(const Vec2& b) const { return {x+b.x, y+b.y}; }
    Vec2 operator-(const Vec2& b) const { return {x-b.x, y-b.y}; }
    Vec2 operator*(double k) const { return {k*x, k*y}; }
    double dot(const Vec2& b) const { return x * b.x + y * b.y; }
    double len() const { return std::sqrt(x*x + y*y); }
    Vec2 norm() const { double l = len(); return (l > 1e-8) ? (*this)*(1.0/l) : Vec2{0,0}; }
};

void to_screen(const Vec2& v, int& sx, int& sy) {
    sx = GRID_ORIGIN_X + int(std::round(v.x * CELL));
    sy = GRID_ORIGIN_Y - int(std::round(v.y * CELL));
}

void drawArrow(SDL_Renderer* ren, const Vec2& start, const Vec2& end, SDL_Color col, int thick=2) {
    int x1, y1, x2, y2;
    to_screen(start, x1, y1);
    to_screen(end, x2, y2);
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int dx = -thick/2; dx <= thick/2; ++dx)
    for (int dy = -thick/2; dy <= thick/2; ++dy)
        SDL_RenderDrawLine(ren, x1+dx, y1+dy, x2+dx, y2+dy);

    Vec2 v = end - start;
    double l = v.len();
    if (l > 0.01) {
        Vec2 unit = v * (1.0 / l);
        Vec2 left = { -unit.y,  unit.x };
        Vec2 right = {  unit.y, -unit.x };
        Vec2 ah1 = end - unit * 0.4 - left * 0.25;
        Vec2 ah2 = end - unit * 0.4 - right * 0.25;
        int hx1, hy1, hx2, hy2, ex, ey;
        to_screen(end, ex, ey);
        to_screen(ah1, hx1, hy1);
        to_screen(ah2, hx2, hy2);
        SDL_RenderDrawLine(ren, ex, ey, hx1, hy1);
        SDL_RenderDrawLine(ren, ex, ey, hx2, hy2);
    }
}

void drawPoint(SDL_Renderer* ren, const Vec2& v, SDL_Color col, int rad=6) {
    int sx, sy;
    to_screen(v, sx, sy);
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    for (int dx=-rad; dx<=rad; ++dx)
    for (int dy=-rad; dy<=rad; ++dy)
        if (dx*dx+dy*dy<=rad*rad)
            SDL_RenderDrawPoint(ren, sx+dx, sy+dy);
}

void renderText(SDL_Renderer* ren, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

// Parses x,y (handles extra spaces, disallows floats or nonsense).
bool parseVec2(const std::string& s, Vec2& out) {
    int x, y;
    std::string noSpace;
    std::copy_if(s.begin(), s.end(), std::back_inserter(noSpace), [](char c){ return !isspace(c); });
    size_t pos = noSpace.find(',');
    if (pos == std::string::npos) return false;
    std::string xs = noSpace.substr(0, pos);
    std::string ys = noSpace.substr(pos+1);
    if (xs.empty() || ys.empty()) return false;
    try {
        size_t idx1, idx2;
        x = std::stoi(xs, &idx1);
        y = std::stoi(ys, &idx2);
        if (idx1 != xs.size() || idx2 != ys.size()) return false; // only full int allowed
    } catch (...) { return false; }
    out.x = x;
    out.y = y;
    // Only allow in-bounds input
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return false;
    return true;
}

bool closeEnough(const Vec2& a, const Vec2& b) {
    return std::abs(a.x-b.x) < 1e-2 && std::abs(a.y-b.y) < 1e-2;
}

Vec2 project(const Vec2& y, const Vec2& u) {
    double k = (y.x*u.x + y.y*u.y) / (u.x*u.x + u.y*u.y);
    return {k*u.x, k*u.y};
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* win = SDL_CreateWindow("Standard Basis Projection Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 20);

    // Load background (optional)
    SDL_Texture* bgTex = nullptr;
    SDL_Surface* bgSurf = IMG_Load("Downloads/Project Template/projection_3d_bg.png");
    if (bgSurf) {
        bgTex = SDL_CreateTextureFromSurface(ren, bgSurf);
        SDL_FreeSurface(bgSurf);
    }

    // STANDARD BASIS:
    Vec2 u1 = {1, 0}; // x-axis
    Vec2 u2 = {0, 1}; // y-axis
    Vec2 u1_vis = u1 * (COLS-1);
    Vec2 u2_vis = u2 * (ROWS-1);
    Vec2 y = {6, 7};

    Uint32 start_time = SDL_GetTicks();
    const Uint32 time_limit = 60 * 1000;

    bool running = true;
    bool show_proj = true;
    bool input_mode = false;
    bool winFlag = false;
    int input_stage = 0; // 0 = none, 1 = y1, 2 = y2, 3 = submitted
    std::string user_input;
    Vec2 user_y1, user_y2;
    std::string input_error;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (!input_mode && input_stage != 3) {
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_LEFT && y.x > 0)  y.x -= 1;
                    if (e.key.keysym.sym == SDLK_RIGHT && y.x < COLS-1) y.x += 1;
                    if (e.key.keysym.sym == SDLK_DOWN && y.y > 0)  y.y -= 1;
                    if (e.key.keysym.sym == SDLK_UP && y.y < ROWS-1)    y.y += 1;
                    if (e.key.keysym.sym == SDLK_SPACE) show_proj = !show_proj;
                    if (e.key.keysym.sym == SDLK_RETURN) { input_mode = true; input_stage = 1; user_input = ""; input_error = ""; }
                }
            } else if (input_mode) { // Input mode!
                if (e.type == SDL_TEXTINPUT) {
                    user_input += e.text.text;
                }
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !user_input.empty()) {
                        user_input.pop_back();
                    }
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        Vec2 parsed;
                        if (parseVec2(user_input, parsed)) {
                            input_error.clear();
                            if (input_stage == 1) {
                                user_y1 = parsed;
                                input_stage = 2;
                                user_input = "";
                            } else if (input_stage == 2) {
                                user_y2 = parsed;
                                input_stage = 3;
                                input_mode = false; // End input
                                // Validate answer
                                Vec2 correct_y1 = project(y, u1);
                                Vec2 correct_y2 = project(y, u2);
                                winFlag = closeEnough(user_y1, correct_y1) && closeEnough(user_y2, correct_y2);
                            }
                        } else {
                            input_error = "Invalid! Use x,y (integers, in bounds)";
                        }
                    }
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        input_mode = false;
                        input_stage = 0;
                        user_input = "";
                        input_error.clear();
                    }
                }
            }
        }

        // Timer
        Uint32 now = SDL_GetTicks();
        int sec_left = int((time_limit - (now - start_time)) / 1000);
        if (sec_left < 0) sec_left = 0;
        bool time_up = (now - start_time) > time_limit;

        Vec2 y1 = project(y, u1);
        Vec2 y2 = project(y, u2);

        // Draw background
        if (bgTex) {
            SDL_Rect bgRect = {0, 0, WIDTH, HEIGHT};
            SDL_RenderCopy(ren, bgTex, NULL, &bgRect);
        } else {
            SDL_SetRenderDrawColor(ren, 15, 20, 30, 255);
            SDL_RenderClear(ren);
        }

        // Draw grid
        SDL_SetRenderDrawColor(ren, 100, 110, 160, 80);
        for (int x = 0; x < COLS+1; ++x)
            SDL_RenderDrawLine(ren, GRID_ORIGIN_X + x*CELL, GRID_ORIGIN_Y, GRID_ORIGIN_X + x*CELL, GRID_ORIGIN_Y - ROWS*CELL);
        for (int yidx = 0; yidx < ROWS+1; ++yidx)
            SDL_RenderDrawLine(ren, GRID_ORIGIN_X, GRID_ORIGIN_Y - yidx*CELL, GRID_ORIGIN_X + COLS*CELL, GRID_ORIGIN_Y - yidx*CELL);

        // Axis labels
        for (int x = 0; x < COLS; ++x)
            renderText(ren, font, std::to_string(x), {170,170,255,180}, GRID_ORIGIN_X + x*CELL + 10, GRID_ORIGIN_Y + 8);
        for (int yidx = 0; yidx < ROWS; ++yidx)
            renderText(ren, font, std::to_string(yidx), {170,170,255,180}, GRID_ORIGIN_X - 28, GRID_ORIGIN_Y - yidx*CELL - 3);

        // Draw axes with arrowheads and label
        drawArrow(ren, {0,0}, {COLS-1,0}, {240,240,255,255}, 6);
        drawArrow(ren, {0,0}, {0,ROWS-1}, {240,240,255,255}, 6);
        renderText(ren, font, "x", {220,220,255,255}, GRID_ORIGIN_X + (COLS-1)*CELL + 20, GRID_ORIGIN_Y + 10);
        renderText(ren, font, "y", {220,220,255,255}, GRID_ORIGIN_X - 18, GRID_ORIGIN_Y - (ROWS-1)*CELL - 20);
        renderText(ren, font, "O", {255,255,255,220}, GRID_ORIGIN_X-25, GRID_ORIGIN_Y+8);

        // Draw basis vectors with big arrowheads
        drawArrow(ren, {0,0}, u1_vis, {255,120,40,255}, 8);
        drawArrow(ren, {0,0}, u2_vis, {60,200,255,255}, 8);
        renderText(ren, font, "u1", {255,120,40,255}, GRID_ORIGIN_X + int(u1_vis.x*CELL) + 15, GRID_ORIGIN_Y - int(u1_vis.y*CELL) - 25);
        renderText(ren, font, "u2", {60,200,255,255}, GRID_ORIGIN_X + int(u2_vis.x*CELL) + 15, GRID_ORIGIN_Y - int(u2_vis.y*CELL) - 25);

        // y vector (player)
        drawArrow(ren, {0,0}, y, {90,255,100,255}, 5);
        drawPoint(ren, y, {90,255,100,255}, 9);
        renderText(ren, font, "y", {90,255,100,255}, GRID_ORIGIN_X + int(y.x*CELL)+13, GRID_ORIGIN_Y - int(y.y*CELL) - 22);

        if (show_proj) {
            drawPoint(ren, y1, {255,120,40,255}, 10);
            drawPoint(ren, y2, {60,200,255,255}, 10);

            int ax, ay, bx, by, cx, cy;
            to_screen(y1, ax, ay);
            to_screen(y, bx, by);
            to_screen(y2, cx, cy);
            SDL_SetRenderDrawColor(ren, 220,220,220,90);
            SDL_RenderDrawLine(ren, ax, ay, bx, by);
            SDL_RenderDrawLine(ren, cx, cy, bx, by);
            SDL_RenderDrawLine(ren, ax, ay, cx, cy);

            renderText(ren, font, "y1: proj onto u1", {255,120,40,255}, WIDTH-340, 70);
            renderText(ren, font, "y2: proj onto u2", {60,200,255,255}, WIDTH-340, 100);
            renderText(ren, font, "ENTER: submit projection coords", {220,220,220,120}, 32, 32);
        } else {
            renderText(ren, font, "SPACE: show projections", {220,220,220,120}, 32, 32);
        }

        // Timer bar
        if (!time_up) {
            int bar_w = int(((time_limit - (now - start_time)) * (WIDTH - 140)) / double(time_limit));
            SDL_SetRenderDrawColor(ren, 0, 200, 255, 230);
            SDL_Rect bar = {80, 14, bar_w, 16};
            SDL_RenderFillRect(ren, &bar);
            std::string timer_str = "Time left: " + std::to_string(sec_left) + "s";
            renderText(ren, font, timer_str, {255,255,255,255}, WIDTH-170, 12);
        } else {
            renderText(ren, font, "Time's Up!", {255, 60, 60, 255}, WIDTH/2-70, 16);
        }

        std::ostringstream oss;
        oss << "y = (" << int(y.x) << ", " << int(y.y) << ")";
        renderText(ren, font, oss.str(), {220,255,180,255}, WIDTH-340, 150);

        // Input mode with robust message and error
        if (input_mode) {
            SDL_Rect overlay = {100, 220, WIDTH-200, 160};
            SDL_SetRenderDrawColor(ren, 30,30,50,210);
            SDL_RenderFillRect(ren, &overlay);
            renderText(ren, font, "Enter y1 and y2 projection coords as x,y", {255,255,255,255}, 120, 240);
            if (input_stage == 1)
                renderText(ren, font, "y1 (onto u1): ", {255,120,40,255}, 120, 270);
            else
                renderText(ren, font, "y2 (onto u2): ", {60,200,255,255}, 120, 270);

            renderText(ren, font, user_input+"|", {250,250,180,255}, 320, 270);
            renderText(ren, font, "(format: x,y)", {210,210,210,120}, 520, 270);
            if (!input_error.empty())
                renderText(ren, font, input_error, {255,50,80,255}, 120, 310);
            renderText(ren, font, "ESC to cancel", {150,160,180,120}, 120, 350);
        }

        // WIN/LOSE message and auto-exit
        if (input_stage == 3) {
            SDL_Rect overlay = {WIDTH/2-150, HEIGHT/2-40, 300, 90};
            SDL_SetRenderDrawColor(ren, winFlag ? 10 : 60, winFlag ? 120 : 30, winFlag ? 30 : 50, 210);
            SDL_RenderFillRect(ren, &overlay);
            if (winFlag) {
                renderText(ren, font, "WIN! Projections are correct!", {255,255,120,255}, WIDTH/2-120, HEIGHT/2-10);
                SDL_RenderPresent(ren);
                SDL_Delay(1800); // Show the win message for 1.8 seconds
                running = false; // End the game loop
                continue;
            } else {
                renderText(ren, font, "Incorrect. Try Again!", {255,80,80,255}, WIDTH/2-120, HEIGHT/2-10);
                renderText(ren, font, "Press ENTER to continue", {220,220,220,180}, WIDTH/2-120, HEIGHT/2+20);
                SDL_Event e2;
                while (SDL_PollEvent(&e2)) {
                    if (e2.type == SDL_KEYDOWN && e2.key.keysym.sym == SDLK_RETURN) {
                        input_stage = 0;
                        winFlag = false;
                    }
                }
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);

        // Enable text input in input mode only
        if (input_mode && !SDL_IsTextInputActive())
            SDL_StartTextInput();
        if (!input_mode && SDL_IsTextInputActive())
            SDL_StopTextInput();
    }

    if (bgTex) SDL_DestroyTexture(bgTex);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
