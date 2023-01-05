#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>

#include <SDL.h>
#include <SDL_ttf.h>

#include "parser.h"

class Buffer
{
public:
	Buffer();

	Buffer(const std::string filename);

	void openFile(const std::string filename);

	void deleteChar(const int row, const int col, int count = 1);

	void deleteAt(const int row, const int col);

	void append(const int row, const int col, const std::string& str);

	void appendNewLine(const int col, const int row);

	size_t getLineSize(const size_t col);

	void saveBuffer();

	size_t size();

	~Buffer();

	void draw(SDL_Renderer* renderer, TTF_Font* font, int& begin_offset, int& end_offset, int col, int col_offset, int cursor_y, int max_cols, Theme theme);

private:
	std::string m_filename;
	std::vector <std::string> m_buffer;

};
