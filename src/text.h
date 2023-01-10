#pragma once

#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

SDL_Surface* makeTextSuface(const std::string& text, uint32_t color);

SDL_Surface* makeSurface(const int width, const int height);

void appendTextToSurface(SDL_Surface* src, SDL_Surface* dst, const int dst_x);

SDL_Texture* makeTextTexture(SDL_Surface* surface);

SDL_Rect makeTextRect(const int x, const int y, SDL_Surface* surface);

SDL_Texture* makeTextTexture(const std::string& text, SDL_Rect& rect, const uint32_t color);

void drawText(const std::string& text, const int x, const int y, const uint32_t color);

void drawText(SDL_Texture* texture, SDL_Rect rect);

enum ColorMask
{
	BLUE,
	GREEN = 8,
	RED   = 16
};

class Text
{
public:
	Text(const std::string& text, int x, int y, uint32_t color);
	Text(int x, int y);
	~Text();
	
	void reserveSurface(const int width, const int height);

	void makeTexture();

	void append(const std::string& text, const uint32_t color);

	SDL_Rect getRect();

	SDL_Surface* getSurface();

	SDL_Texture* getTexture();

	void draw();

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
