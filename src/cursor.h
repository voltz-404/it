#pragma once

#include "pch.h"

class Buffer;

class Cursor
{
public:
    Cursor();

    void moveDown(size_t buffer_max_col);

    void moveUp();

    void moveLeft();

    void moveRight(size_t line_max_row);

    size_t row();

    size_t col();

    int x();

    int y();

    int width();

    int height();

    void move(size_t row, size_t col);

    void update(Buffer buffer);

    void draw();

private:
    size_t m_row;
    size_t m_col;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
};