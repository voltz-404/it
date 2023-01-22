#include "internal.h"

#include <iostream>

Editor::Editor()
{
    m_renderer = nullptr;
    m_window = nullptr;
    m_font = nullptr;
    m_screen_width = 1080;
    m_screen_height = 720;

    keys = SDL_GetKeyboardState(nullptr);
}

Editor* Editor::get()
{
    static Editor* editor;

    if (editor == nullptr)
    {
        editor = new Editor();
        editor->m_window = SDL_CreateWindow("Blaze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,1080, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        if (editor->m_window == nullptr)
        {
            std::cerr << "Couldn't create the window\n";
            return nullptr;
        }
        editor->m_renderer = SDL_CreateRenderer(editor->m_window, -1, SDL_RENDERER_ACCELERATED);
        if (editor->m_renderer == nullptr)
        {
            std::cerr << "Couldn't create the renderer: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(editor->m_window);
            return nullptr;
        }
        editor->m_font = TTF_OpenFont("./assets/fonts/Inconsolata-Regular.ttf", 18);
        if (editor->m_font == nullptr)
        {
            printf("%s\n", TTF_GetError());
            SDL_DestroyRenderer(editor->m_renderer);
            SDL_DestroyWindow(editor->m_window);
            SDL_Quit();

            return nullptr;
        }
    }
    return editor;
}

SDL_Renderer* Editor::getRenderer()
{
    return get()->m_renderer;
}

SDL_Window* Editor::getWindow()
{
    return get()->m_window;
}

TTF_Font* Editor::getFont()
{
    return get()->m_font;
}

int Editor::getScreenWidth()
{
    return get()->m_screen_width;
}

int Editor::getScreenHeight()
{
    return get()->m_screen_height;
}

int Editor::getScreenCols()
{
    int glyph_height = 0;
    TTF_SizeText(Editor::getFont(), "A", nullptr, &glyph_height);
    return get()->m_screen_height / glyph_height;
}

int Editor::getScreenRows()
{
    int glyph_width = 0;
    TTF_SizeText(Editor::getFont(), "A", &glyph_width, nullptr);
    return get()->m_screen_width / glyph_width;
}

bool Editor::isKeyDown(SDL_Scancode key)
{
    return get()->keys[key];
}

bool ctrlKey(SDL_Scancode key)
{
    return Editor::isKeyDown(SDL_SCANCODE_LCTRL) && Editor::isKeyDown(key);
}