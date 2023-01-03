#include "cursor.h"

#include <stdio.h>

Cursor::Cursor(int width, int height, int max_row, int max_col)
{
	m_max_col = max_col;
	m_max_row = max_row;
	m_row = 1;
	m_col = 1;
	m_x = 0;
	m_y = 0;
	m_width = width;
	m_height = height;
}

void Cursor::moveDown(int buffer_max_col)
{
	if (m_col < buffer_max_col)
	{
		m_col += 1;

		m_x = 0;
		m_row = 1;

		if (m_y < (m_max_col - 1) * m_height)
			m_y += m_height;
	}
}

void Cursor::moveUp()
{
	if (m_col > 1)
	{
		m_col -= 1;

		// TODO: weird cursor position when moving up
		if (m_y - m_height >= 0)
			m_y -= m_height;
	}
}

void Cursor::moveLeft()
{
	if (m_row > 1)
	{
		m_x -= m_width;
		m_row -= 1;
	}
}

void Cursor::moveRight(int line_max_row)
{
	if (m_row < line_max_row + 1)
	{
		m_row += 1;

		if (m_row < m_max_row)
			m_x += m_width;
	}
}

int Cursor::row()
{
	return m_row;
}

int Cursor::col()
{
	return m_col;
}

int Cursor::x()
{
	return m_x;
}

int Cursor::y()
{
	return m_y;
}

void Cursor::move(int row, int col)
{
	m_row = row;
	m_col = col;

	m_x = (m_row - 1) * m_width;
	m_y = (m_col - 1) * m_height;
}

void Cursor::draw(SDL_Renderer* renderer)
{
	SDL_Color cursor_color = { 0x07, 0x66, 0x78, 0xFF };
	SDL_Rect cursor_rect = { m_x, m_y, m_width, m_height };
	SDL_SetRenderDrawColor(renderer, cursor_color.r, cursor_color.g, cursor_color.b, 0xff);
	SDL_RenderDrawLine(renderer, m_x, m_y, m_x, m_y + m_height);
}

