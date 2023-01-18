#include "buffer.h"

#include <stdio.h>
#include <algorithm>

#include "text.h"
#include "platform.h"
#include "internal.h"

Buffer::Buffer()
{
    m_filename = "";
    m_buffer = { "" };
    m_textures = { nullptr };
    m_redraw = true;
    m_file_saved = false;
}

Buffer::Buffer(const std::string& filename)
{
	m_filename = filename;
	m_buffer = { "" };
    m_textures = { nullptr };
    m_redraw = true;
    m_file_saved = false;

    if (filename.size() != 0)
	    openFile(filename);
}

void Buffer::openFile(const std::string& filename)
{
    if (filename.size() < 1) return;

    std::ifstream file(filename);
    if (file)
    {
        if (m_buffer.size() > 0)
        {
            m_buffer.clear();
        }

        std::string line;
        while (getline(file, line))
        {
            if (line.size() > 0 && line[0] == '\t')
            {
                line.erase(line.begin());
                line.insert(0, "    ");
            }
            m_buffer.emplace_back(line);
        }

        m_filename = filename;
        m_file_saved = true;
    }
    else
    {
        m_buffer = { "" };
        printf("Error: %s\n", strerror(errno));
        printf("Could not open the file: %s\n", filename.c_str());
    }

    m_redraw = true;
}

void Buffer::deleteLine(const size_t col)
{
    if (col == 1)
    {
        m_buffer[col - 1] = "";
    }
    else
    {
        m_buffer.erase(m_buffer.begin() + (col - 1));
    }

    m_redraw = true;
}

void Buffer::deleteAt(const size_t row, const size_t col)
{
    if (m_buffer.size() >= 1 && row > 1)
    {
        m_buffer[col - 1].erase(row - 2, 1);
    }
    else if (col > 1)
    {
        m_buffer[col - 2] += m_buffer[col - 1];
        m_buffer.erase(m_buffer.begin() + (col - 1));
    }

    m_redraw = true;
    m_file_saved = false;
}

void Buffer::append(const size_t row, const size_t col, const std::string_view& str)
{
    if (m_buffer.size() != 0)
    {
        m_buffer[col].insert(row, str);
    }
    else
        m_buffer.emplace_back(str);

    m_redraw = true;
    m_file_saved = false;
}

void Buffer::appendNewLine(const size_t col, const size_t row)
{
    if (m_buffer.size() >= 1 && m_buffer[col - 1 ].size() > row - 1)
    {
        std::string new_line = m_buffer.at(col - 1).substr(row - 1);
        m_buffer.at(col - 1).erase(row - 1);

        m_buffer.insert(m_buffer.begin() + col, new_line);
    }
    else
    {
        m_buffer.insert(m_buffer.begin() + col, "");
    }

    //if (row > 1)
    //{
    //    if (m_buffer[col - 1][row - 2] == '{')
    //    {
    //        m_buffer.insert(m_buffer.begin() + (col), "");
    //    }
    //}

    m_redraw = true;
    m_file_saved = false;
}

size_t Buffer::getLineSize(const size_t col)
{
    return m_buffer[col].size();
}

void Buffer::saveBuffer()
{
    std::stringstream file_content;
    for (const std::string_view& line : m_buffer)
    {
        file_content << line << '\n';
    }

    // If files has no name as for one
    if (m_filename.size() == 0)
        m_filename = getSaveFileName();

    std::ofstream file(m_filename);
    if (file.is_open())
    {
        file << file_content.str();
        m_file_saved = true;
    }
}

std::string Buffer::getFilename()
{
    return m_filename;
}

void Buffer::redraw()
{
    m_redraw = true;
}

size_t Buffer::size()
{
    return m_buffer.size();
}

bool Buffer::fileSaved()
{
    return m_file_saved;
}

Buffer::~Buffer()
{
}

SDL_Texture* renderLine(const std::string& line, int x, int y, Theme theme)
{
    if (line.size() < 1) return nullptr;
    std::vector<Token> tokens = parser(line);

    const std::vector<std::string> keywords = { "int", "string", "return", "void", "char", "uint_32", "if", "else", "while", "for" ,"switch", "include", "const", "def", "import" };

    int line_w = 0, line_h = 0;
    TTF_SizeText(Editor::getFont(), line.c_str(), &line_w, &line_h);

    line_w += 3;

    // Surface with alocated space to fit all words in their respective colors
    SDL_Surface* text_surface = makeSurface(line_w, line_h);

    int text_surface_pos = 0;
    bool string_start = false;
    for (const Token& token : tokens)
    {
        uint32_t color;
        int width = 0, height = 0;
        TTF_SizeText(Editor::getFont(), token.str.c_str(), &width, &height);

        if (token.str == "\"")
        {
            color = theme.string_;
            string_start = !string_start;
        }
        else if (token.str == "#")
            color = theme.namepce;
        else if (std::find(keywords.begin(), keywords.end(), token.str) != keywords.end())
            color = theme.keyword;
        else if (token.type == Token::Type::NUM)
            color = theme.numbers;
        else if (token.str == "std")
            color = theme.namepce;
        else
            color = theme.symbols;

        if (string_start)
        {
            color = theme.string_;
        }

        if (!string_start && token.str == "\"")
        {
            color = theme.string_;
        }

        // Add Word on text_surface
        SDL_Surface* surface = makeTextSuface(token.str, color);
        appendTextToSurface(surface, text_surface, text_surface_pos);
        text_surface_pos += surface->w;
        SDL_FreeSurface(surface);
    }

    SDL_Texture* texture = makeTextTexture(text_surface);
    SDL_FreeSurface(text_surface);
    return texture;
}

void Buffer::draw(size_t& begin_offset, size_t& end_offset, size_t col, size_t col_offset, size_t cursor_y, size_t max_cols, Theme theme)
{
    int glyph_height = 0;
    TTF_SizeText(Editor::getFont(), "A", nullptr, &glyph_height);

    if (m_redraw)
    {
        //printf("redraw called: %s\n", m_redraw ? "true" : "false");
        if (cursor_y == 0 && col > 0)
        {
            begin_offset = col - 1;
            end_offset = col + max_cols - 1;
        }
        if (col_offset == max_cols && col > max_cols - 1)
        {
            begin_offset = col - max_cols;
            end_offset = col;
        }

        for (SDL_Texture* texture : m_textures)
            SDL_DestroyTexture(texture);

        m_textures.clear();

        for (size_t i = begin_offset, j = 0; i < end_offset && i < m_buffer.size(); i++, j++)
        {
            SDL_Texture* texture = renderLine(m_buffer[i], 0, j * glyph_height, theme);
            m_textures.push_back(texture);
        }

        m_redraw = false;
    }

    if (!m_redraw)
    {
        for (int i = 0; i < m_textures.size(); i++)
        {
            if (m_textures[i] == nullptr) continue;

            int texture_width = 0;
            SDL_QueryTexture(m_textures[i], nullptr, nullptr, &texture_width, nullptr);
            SDL_Rect rect = { 0, i * glyph_height, texture_width, glyph_height };
            drawText(m_textures[i], rect);
        }
    }
}
