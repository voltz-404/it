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

	Buffer(const std::string& filename);

	void openFile(const std::string& filename);

	void deleteLine(const size_t col);

	void deleteChars(const size_t row, const size_t col, const size_t count = 1);

	void deleteAt(const size_t row, const size_t col, const size_t count = 1);

	void append(const size_t row, const size_t col, const std::string_view& str);

	void appendNewLine(const size_t col, const size_t row);

	void appendLine(const size_t col, const char* line);

	size_t getLineSize(const size_t col);

	std::string getLine(const size_t col);

	void saveBuffer();

	std::string getFilename();

	void redraw();

	size_t size();

	bool fileSaved();

	~Buffer();

	void draw(size_t& begin_offset, size_t& end_offset, size_t col, size_t col_offset, size_t cursor_y, size_t max_cols, Theme theme);

private:
	std::string m_filename;
	std::vector <std::string> m_buffer;
	std::vector <SDL_Texture*> m_textures;

	bool m_redraw;
	bool m_file_saved;
};
