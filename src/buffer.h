#pragma once

#include "pch.h"

#include "parser.h"

#include "cursor.h"

class Selection
{
public:
	int start_row;
	int end_row;

	int start_y;

	int start_col;
	int end_col; // not implemented

	void startSelection(Cursor& cursor);

	bool hasSelection();

	void draw();
};

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

	void appendNewLine(const size_t col, const size_t row, const int tabstop = 0);

	void appendLine(const size_t col, const char* line);

	size_t getLineSize(const size_t col);

	std::string getLine(const size_t col);

	void saveBuffer();

	std::string getFilename();

	void redraw();

	size_t size();

	void update(SDL_Event event);

	bool fileSaved();

	int getCursorRow();

	int getCursorCol();

	void setCursorCol(const size_t col);

	void keyHandler(SDL_Keycode key);

	void draw(size_t& begin_offset, size_t& end_offset, Theme theme);

private:
	std::string m_filename;
	std::vector <std::string> m_buffer;
	std::vector <SDL_Texture*> m_textures;

	Cursor m_cursor;
	Selection m_selection;

	bool m_redraw;
	bool m_file_saved;
};
