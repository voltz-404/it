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

	void deleteChar(const size_t row, const size_t col, int count = 1);

	void deleteAt(const size_t row, const size_t col);

	void append(const size_t row, const size_t col, const std::string& str);

	void appendNewLine(const size_t col, const size_t row);

	size_t getLineSize(const size_t col);

	void saveBuffer();

	void redraw();

	size_t size();

	~Buffer();

	void draw(size_t& begin_offset, size_t& end_offset, size_t col, size_t col_offset, size_t cursor_y, size_t max_cols, Theme theme);

private:
	std::string m_filename;
	std::vector <std::string> m_buffer;
	std::vector <SDL_Texture*> m_textures;

	bool m_redraw;
};
