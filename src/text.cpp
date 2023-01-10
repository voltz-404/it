#include "text.h"

#include "internal.h"

uint8_t getColor(const uint32_t color, ColorMask color_mask)
{
    return (uint8_t)(color >> color_mask & 0xff);
}

SDL_Surface* makeTextSuface(const std::string& text, uint32_t color)
{
    SDL_Surface* surface = nullptr;
    SDL_Color text_color = { getColor(color, RED), getColor(color, GREEN), getColor(color, BLUE), 0xff };
    surface = TTF_RenderUTF8_Blended(Editor::getFont(), text.c_str(), text_color);

    return surface;
}

SDL_Surface* makeSurface(const int width, const int height)
{
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    Uint32 colorkey = SDL_MapRGB(surface->format, 0x0, 0x0, 0x0);
    SDL_SetColorKey(surface, SDL_TRUE, colorkey);

    return surface;
}

void appendTextToSurface(SDL_Surface* src, SDL_Surface* dst, const int dst_x)
{
    SDL_Rect rect = { 0, 0,src->w, src->h };
    SDL_Rect rect_pos = { dst_x, 0, 0, 0 };

    SDL_BlitSurface(src, &rect, dst, &rect_pos);
}

SDL_Texture* makeTextTexture(SDL_Surface* surface)
{
    return SDL_CreateTextureFromSurface(Editor::getRenderer(), surface);
}

SDL_Texture* makeTextTexture(const std::string& text, SDL_Rect& rect, const uint32_t color)
{
    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;

    if ((surface = makeTextSuface(text, color)) == nullptr)
    {
        printf("[Surface] %s\n", TTF_GetError());
        return nullptr;
    }

    rect.w = surface->w;
    rect.h = surface->h;

    texture = makeTextTexture(surface);

    SDL_FreeSurface(surface);

    return texture;
}

SDL_Rect makeTextRect(const int x, const int y, SDL_Surface* surface)
{
    SDL_Rect rect = { x, y, surface->w, surface->h };

    return rect;
}

void drawText(const std::string& text, const int x, const int y, const uint32_t color)
{
    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Rect rect = { 0 };
    if ((surface = makeTextSuface(text, color)) == nullptr)
    {
        printf("[Surface] %s\n", TTF_GetError());
        return;
    }
    if ((texture = makeTextTexture(surface)) == nullptr)
    {
        printf("[Texture] %s\n", SDL_GetError());
        return;
    }

    rect = makeTextRect(x, y, surface);

    SDL_RenderCopy(Editor::getRenderer(), texture, nullptr, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


// Draw a texture to the screen, but does not free it
void drawText(SDL_Texture* texture, SDL_Rect rect)
{
    SDL_RenderCopy(Editor::getRenderer(), texture, nullptr, &rect);
}

Text::Text(const std::string& text, int x, int y, uint32_t color)
{
    if (std::string(text).size() < 1)
        return;

    m_texture = nullptr;
    m_surface = makeTextSuface(text, color);
    m_text = text;
    m_color = 0x0;
    m_font = Editor::getFont();
    m_append_width = 0;
    m_x = x;
    m_y = y;
    m_rect = { x, y, m_surface->w, m_surface->h };
}

Text::Text(int x, int y)
{
    m_texture = nullptr;
    m_surface = nullptr;
    m_text = "";
    m_color = 0x0;
    m_font = Editor::getFont();
    m_append_width = 0;
    m_x = x;
    m_y = y;
    m_rect = { x, y, 0, 0 };
}

void Text::makeTexture()
{
    if ((m_texture = SDL_CreateTextureFromSurface(Editor::getRenderer(), m_surface)) == nullptr)
    {
        printf("Could not creaetd texture from surface: %s\n", TTF_GetError());
        SDL_FreeSurface(m_surface);
        return;
    }
}

// Call Text::reserveSurface(...) before calling this function cuz it assumes the surface has enough width
void Text::append(const std::string& text, const uint32_t color)
{
    SDL_Surface* surface = makeTextSuface(text, color);
    SDL_Rect rect = { 0, 0, surface->w, surface->h };
    SDL_Rect rect_pos = { m_append_width, 0, 0, 0 };

    SDL_BlitSurface(surface, &rect, m_surface, &rect_pos);
    m_append_width += surface->w;

    SDL_FreeSurface(surface);
}

SDL_Rect Text::getRect()
{
    return m_rect;
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

SDL_Texture* Text::getTexture()
{
    return m_texture;
}

void Text::draw()
{
    m_rect = { m_x, m_y, m_surface->w, m_surface->h };
    //SDL_FreeSurface(m_surface);
    SDL_RenderCopy(Editor::getRenderer(), m_texture, nullptr, &m_rect);
}
