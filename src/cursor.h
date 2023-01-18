#pragma once

#include <SDL.h>

class Cursor
{
public:
	Cursor(int width, int height, size_t max_row, size_t max_col);

	void moveDown(size_t buffer_max_col);

	void moveUp();

	void moveLeft();

	void moveRight(size_t line_max_row);

	size_t row();

	size_t col();

	int x();

	int y();

	void move(size_t row, size_t col);

	void draw();

private:
	size_t m_max_col;
	size_t m_max_row;
	size_t m_row;
	size_t m_col;
	int m_x;
	int m_y;
	int m_width;
	int m_height;
};