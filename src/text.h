#pragma once

#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

enum ColorMask
{
	BLUE,
	GREEN = 8,
	RED   = 16
};

class Text
{
public:
	Text(const std::string& text, int x, int y, TTF_Font* font, uint32_t color);
	Text(TTF_Font* font, int x, int y);
	~Text();
	
	void reserveSurface(const int width, const int height);

	void makeTexture(SDL_Renderer* renderer);

	void append(const std::string& text, const uint32_t color);

	SDL_Rect getRect();

	SDL_Surface* getSurface();

	void draw(SDL_Renderer* renderer);
private:
	SDL_Surface* m_surface;
	SDL_Texture* m_texture;
	SDL_Rect m_rect;
	int m_x;
	int m_y;
	int m_append_width;
	std::string m_text;
	uint32_t m_color;
	TTF_Font* m_font;
};
