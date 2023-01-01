#include "text.h"


uint8_t getColor(const uint32_t color, ColorMask color_mask)
{
    return (uint8_t)(color >> color_mask & 0xff);
}

SDL_Surface* makeSurfaceFromText(const std::string& text, TTF_Font* font, uint32_t color)
{
    SDL_Surface* surface = nullptr;
    SDL_Color text_color = { getColor(color, RED), getColor(color, GREEN), getColor(color, BLUE), 0xff };
    if ((surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), text_color, 0)) == nullptr)
    {
        printf("Could not creaetd surface from font: %s\n", TTF_GetError());
    }

    return surface;
}

Text::Text(const std::string& text, int x, int y, TTF_Font* font, uint32_t color)
{
    if (std::string(text).size() < 1)
        return;

    m_texture = nullptr;
    m_surface = makeSurfaceFromText(text, font, color);
    m_text = text;
    m_color = 0x0;
    m_font = font;
    m_append_width = 0;
    m_x = x;
    m_y = y;
    m_rect = { x, y, m_surface->w, m_surface->h };
}

Text::Text(TTF_Font* font, int x, int y)
{
    m_texture = nullptr;
    m_surface = nullptr;
    m_text = "";
    m_color = 0x0;
    m_font = font;
    m_append_width = 0;
    m_x = x;
    m_y = y;
    m_rect = { x, y, 0, 0 };
}

void Text::makeTexture(SDL_Renderer* renderer)
{
    if ((m_texture = SDL_CreateTextureFromSurface(renderer, m_surface)) == nullptr)
    {
        printf("Could not creaetd texture from surface: %s\n", TTF_GetError());
        SDL_FreeSurface(m_surface);
        return;
    }

}

// Call Text::reserveSurface(...) before calling this function cuz it assumes the surface has enough width
void Text::append(const std::string& text, const uint32_t color)
{
    SDL_Surface* surface = makeSurfaceFromText(text, m_font, color);
    SDL_Rect rect = { 0, 0, surface->w, surface->h };
    SDL_Rect rect_pos = { m_append_width, 0, 0, 0 };

    SDL_BlitSurface(surface, &rect, m_surface, &rect_pos);
    m_append_width += surface->w;

    SDL_FreeSurface(surface);
}

Text::~Text()
{
    SDL_FreeSurface(m_surface);

    SDL_DestroyTexture(m_texture);
}

// Erases the current surface and allocate a new with the width and height specified
void Text::reserveSurface(const int width, const int height)
{

    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    SDL_FreeSurface(m_surface);
    m_surface = surface;
    Uint32 colorkey = SDL_MapRGB(m_surface->format, 0x0, 0x0, 0x0);
    SDL_SetColorKey(m_surface, SDL_TRUE, colorkey);
}

void Text::draw(SDL_Renderer* renderer)
{
    m_rect = { m_x, m_y, m_surface->w, m_surface->h };
    //SDL_FreeSurface(m_surface);
    SDL_RenderCopy(renderer, m_texture, nullptr, &m_rect);
}
