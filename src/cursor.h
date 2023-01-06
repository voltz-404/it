#pragma once

#include <SDL.h>

class Cursor
{
public:
	Cursor(int width, int height, int max_row, int max_col);

	void moveDown(size_t buffer_max_col);

	void moveUp();

	void moveLeft();

	void moveRight(size_t line_max_row);

	int row();

	int col();

	int x();

	int y();

	void move(int row, int col);

	void draw();

private:
	int m_max_col;
	int m_max_row;
	int m_row;
	int m_col;
	int m_x;
	int m_y;
	int m_width;
	int m_height;
};