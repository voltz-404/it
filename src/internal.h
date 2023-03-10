#pragma once

#include "pch.h"

class Editor
{
public:
    static Editor* get();

    static SDL_Renderer* getRenderer();

    static SDL_Window* getWindow();

    static TTF_Font* getFont();

    static int getScreenWidth();

    static int getScreenHeight();

    static int getScreenCols();
        
    static int getScreenRows();

    static bool isKeyDown(SDL_Scancode key);

private:
    Editor();
    Editor(Editor& other) = delete;

    static Editor* m_instace;

    SDL_Renderer* m_renderer;
    SDL_Window* m_window;
    TTF_Font* m_font;
    int m_screen_width;
    int m_screen_height;

    const Uint8* keys;
};

bool ctrlKey(SDL_Scancode key);