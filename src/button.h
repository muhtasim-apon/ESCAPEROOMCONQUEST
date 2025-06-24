// src/button.h
#pragma once
#include <SDL2/SDL.h>
#include <string>

struct Button {
    SDL_Rect    rect;
    std::string label;
    SDL_Color   color;
    bool        isHovered = false;

    Button(int x,int y,int w,int h,const std::string& txt, SDL_Color col)
      : rect{x,y,w,h}, label(txt), color(col) {}
};
