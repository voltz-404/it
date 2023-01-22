#include "cursor.h"

#include <stdio.h>

#include "internal.h"
#include "buffer.h"

Cursor::Cursor()
{
    m_max_col = Editor::getScreenCols();
    m_max_row = Editor::getScreenRows();
    m_row = 1;
    m_col = 1;
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;

    TTF_SizeText(Editor::getFont(), "_", &m_width, &m_height);
}

void Cursor::moveDown(size_t buffer_max_col)
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

        m_x = 0;
        m_row = 1;

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

void Cursor::moveRight(size_t line_max_row)
{
    if (m_row - 1 < line_max_row)
    {
        m_row += 1;

        if (m_row < m_max_row)
            m_x += m_width;
    }
}

size_t Cursor::row()
{
    return m_row;
}

size_t Cursor::col()
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

int Cursor::width()
{
    return m_width;
}

int Cursor::height()
{
    return m_height;
}

void Cursor::move(size_t row, size_t col)
{
    if (row > 0)
    {
        m_row = row;
        m_x = ((row - 1) % Editor::getScreenRows()) * m_width;
    }

    if (col > 0)
    {
        m_col = col;
        m_y = 0;
    }
}

void Cursor::update(Buffer buffer)
{
}

void Cursor::draw()
{
    SDL_Color cursor_color = { 0xFF, 0xB4, 0x54, 0xFF };
    SDL_Rect cursor_rect = { m_x, m_y, m_width, m_height };
    SDL_SetRenderDrawColor(Editor::getRenderer(), cursor_color.r, cursor_color.g, cursor_color.b, 0xff);
    SDL_RenderDrawLine(Editor::getRenderer(), m_x, m_y, m_x, m_y + m_height);
    //SDL_RenderFillRect(Editor::getRenderer(), &cursor_rect);
}


