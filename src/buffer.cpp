#include "buffer.h"

#include "text.h"
#include "platform.h"
#include "internal.h"


Buffer::Buffer()
    : m_cursor()
{
    m_filename = "";
    m_buffer = { "" };
    m_textures = { nullptr };
    m_redraw = true;
    m_file_saved = false;
    m_selection = { 0 };
}

Buffer::Buffer(const std::string& filename)
    : m_cursor()
{
    m_filename = filename;
    m_buffer = { "" };
    m_textures = { nullptr };
    m_redraw = true;
    m_file_saved = false;
    m_selection = { 0 };

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
        m_filename = "";
        m_file_saved = false;
        printf("Error: %s\n", strerror(errno));
        printf("Could not open the file: %s\n", filename.c_str());
    }

    file.close();

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

void Buffer::deleteAt(const size_t row, const size_t col, const size_t count)
{
    if (m_buffer.size() >= 1 && row > 1)
    {
        m_buffer[col - 1].erase(row - 2, count);
    }
    else if (col > 1)
    {
        m_buffer[col - 2] += m_buffer[col - 1];
        m_buffer.erase(m_buffer.begin() + (col - 1));
    }

    m_redraw = true;
    m_file_saved = false;
}

void Buffer::deleteChars(const size_t row, const size_t col, const size_t count)
{
    m_buffer[col].erase(row, count);
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

void Buffer::appendNewLine(const size_t col, const size_t row, const int tabstop)
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

    const std::string spaces(tabstop, ' ');

    m_buffer[col].insert(0, spaces);


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

void Buffer::appendLine(const size_t col, const char* line)
{
    m_buffer.insert(m_buffer.begin() + col, line);
    redraw();
}

std::string Buffer::getLine(const size_t col)
{
    return m_buffer[col - 1];
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

        if (token.str == "/")
        {
            color = theme.string_;
            string_start = true;
        }
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

void Buffer::draw(size_t& begin_offset, size_t& end_offset, Theme theme)
{
    int glyph_height = 0;
    TTF_SizeText(Editor::getFont(), "A", nullptr, &glyph_height);
    size_t col = m_cursor.col();
    size_t cursor_y = m_cursor.y();
    size_t max_cols = Editor::getScreenCols();
    int col_offset = m_cursor.y() > 0 ? m_cursor.y() / glyph_height + 1 : 1;

    m_cursor.draw();

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

        for (int i = begin_offset, j = 0; i < end_offset && i < m_buffer.size(); i++, j++)
        {
            SDL_Texture* texture = renderLine(m_buffer[i], 0, j * glyph_height, theme);
            m_textures.push_back(texture);
        }

        m_redraw = false;
    }

    if (!m_redraw)
    {
        SDL_Rect line_highlight = { 0, m_cursor.y(), Editor::getScreenWidth(), glyph_height};
        SDL_SetRenderDrawColor(Editor::getRenderer(), 0x69, 0x81, 0xfa, 0xa);
        SDL_RenderFillRect(Editor::getRenderer(), &line_highlight);
        for (int i = 0; i < m_textures.size(); i++)
        {
            if (m_textures[i] == nullptr) continue;

            int texture_width = 0;
            SDL_QueryTexture(m_textures[i], nullptr, nullptr, &texture_width, nullptr);
            SDL_Rect rect = { 0, i * glyph_height, texture_width, glyph_height };
            drawText(m_textures[i], rect);
        }
    }


    if (m_selection.hasSelection())
        m_selection.draw();
}

void Buffer::update(SDL_Event event)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
    {
        // CTRL-key shortcuts
        {
            if (ctrlKey(SDL_SCANCODE_D))
            {
                appendLine(m_cursor.col() - 1, m_buffer[m_cursor.col() - 1].c_str());
                m_cursor.moveDown(m_buffer.size());
            }
            else if (ctrlKey(SDL_SCANCODE_O))
            {
                std::string filename = getOpenFileName();
                if (filename.size() > 0)
                {
                    m_cursor.move(1, 1);
                    openFile(filename);
                }
            }
            else if (ctrlKey(SDL_SCANCODE_S))
            {
                saveBuffer();
            }
            else if (ctrlKey(SDL_SCANCODE_L))
            {
                m_cursor.move(m_buffer[m_cursor.col() - 1].size() + 1, 0);
            }
            else if (ctrlKey(SDL_SCANCODE_J))
            {
                m_cursor.move(1, 0);
            }
            else if (ctrlKey(SDL_SCANCODE_LEFTBRACKET))
            {
                append(0, m_cursor.col() - 1, "    ");
                m_cursor.move(m_cursor.row() + 4, 0);
            }
            else if (ctrlKey(SDL_SCANCODE_X))
            {
                deleteLine(m_cursor.col());
                m_cursor.move(1, 0);
                if (m_cursor.col() > size())
                    m_cursor.moveUp();
            }
            else if (ctrlKey(SDL_SCANCODE_V))
            {
                std::string clipboard = SDL_GetClipboardText();
                append(m_cursor.row() - 1, m_cursor.col() - 1, clipboard);
                m_cursor.move(m_cursor.row() + clipboard.size(), 0);
            }
            
            if (Editor::isKeyDown(SDL_SCANCODE_LSHIFT) && Editor::isKeyDown(SDL_SCANCODE_RIGHT))
            {
                m_selection.startSelection(m_cursor);

                if (m_selection.end_row < m_buffer[m_cursor.col() - 1].size() + 1)
                    m_selection.end_row++;

            }
            
            if (Editor::isKeyDown(SDL_SCANCODE_LSHIFT) && Editor::isKeyDown(SDL_SCANCODE_LEFT))
            {
                m_selection.startSelection(m_cursor);
                if (m_cursor.row() > 1)
                    m_selection.end_row--;
            }
        }
        // CTRL-key shortcuts

        keyHandler(event.key.keysym.sym);
    }
    break;
    case SDL_TEXTINPUT:
    {
        if (event.text.text[0] == '{')
        {
            append(m_cursor.row() - 1, m_cursor.col() - 1, "{}");
        }
        else if (std::string(event.text.text) == "(")
        {
            append(m_cursor.row() - 1, m_cursor.col() - 1, "()");
        }
        else if (std::string(event.text.text) == "[")
        {
            append(m_cursor.row() - 1, m_cursor.col() - 1, "[]");
        }
        else if (std::string(event.text.text) == "\"")
        {
            append(m_cursor.row() - 1, m_cursor.col() - 1, "\"\"");
        }
        else
            append(m_cursor.row() - 1, m_cursor.col() - 1, event.text.text);

        m_cursor.moveRight(m_buffer[m_cursor.col() - 1].size());
    }
    break;
    case SDL_MOUSEWHEEL:
    {
        // Mouse scroll up
        if (event.wheel.y > 0)
        {
            redraw();
            m_cursor.move(1, m_cursor.col() - 1);
        }

        // Mouse scroll down
        if (event.wheel.y < 0 && m_cursor.col() < m_buffer.size())
        {
            redraw();
            m_cursor.move(1, m_cursor.col() + 1);
        }
    }
    break;
    default:
        break;
    }
}

int Buffer::getCursorRow()
{
    return m_cursor.row();
}

int Buffer::getCursorCol()
{
    return m_cursor.col();
}

void Buffer::setCursorCol(const size_t col)
{
    m_cursor.move(1, col);
}

void Buffer::keyHandler(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_DOWN:
    {
        size_t cursor_row_pos = m_cursor.row();

        if (m_cursor.col() > Editor::getScreenCols() - 1 && m_cursor.y() + m_cursor.height() >= Editor::getScreenCols() * m_cursor.height())
        {
            redraw();
        }

        m_cursor.moveDown(m_buffer.size());

        if (m_buffer[m_cursor.col() - 1].size() > cursor_row_pos - 1)
            m_cursor.move(cursor_row_pos, 0);
        else
            m_cursor.move(m_buffer[m_cursor.col() - 1].size() + 1, 0);
    }
    break;
    case SDLK_LEFT:
    {
        m_cursor.moveLeft();
    }
    break;
    case SDLK_RIGHT:
    {
        m_cursor.moveRight(m_buffer[m_cursor.col() - 1].size());
    }
    break;
    case SDLK_UP:
    {
        size_t cursor_row_pos = m_cursor.row();

        if (m_cursor.y() == 0 && m_cursor.col() > 1)
            redraw();
        m_cursor.moveUp();

        if (getLineSize(m_cursor.col() - 1) > cursor_row_pos)
            m_cursor.move(cursor_row_pos, 0);
        else
            m_cursor.move(m_buffer[m_cursor.col() - 1].size() + 1, 0);
    }
    break;
    case SDLK_KP_ENTER:
    case SDLK_RETURN:
    {
        // indentation
        int tabstop = 0;
        for (; tabstop < m_buffer[m_cursor.col() - 1].size() && m_buffer[m_cursor.col() - 1][tabstop] == ' '; tabstop++);

        appendNewLine(m_cursor.col(), m_cursor.row(), tabstop);
        m_cursor.moveDown(m_buffer.size());
        // make cursor be in sync with indentation
        m_cursor.move(m_cursor.row() + tabstop, 0);
    }
    break;
    case SDLK_BACKSPACE:
    {
        if (!m_selection.hasSelection())
        {
            // move cursor to end of the above line when delete from row = 1
            size_t cursor_row = m_cursor.row();
            if (m_cursor.col() > 1)
                cursor_row = m_buffer[m_cursor.col() - 2].size() + 2;

            deleteAt(m_cursor.row(), m_cursor.col());

            if (m_buffer.size() >= 1 && m_cursor.row() == 1 && m_cursor.col() > 1)
            {
                m_cursor.moveUp();
                m_cursor.move(cursor_row, 0);
            }

            m_cursor.moveLeft();
        }
        else
        {
            // if selecting to right
            if (m_selection.end_row > m_selection.start_row)
            {
                m_cursor.move(m_selection.start_row, 0);
                deleteAt(m_selection.start_row + 1, m_selection.start_col, m_selection.end_row - m_selection.start_row);
            }
            else // if selecting to left
            {
                m_cursor.move(m_selection.end_row, 0);
                deleteAt(m_selection.end_row + 1, m_selection.start_col, m_selection.start_row - m_selection.end_row);
            }

            m_selection.end_row = m_selection.start_row;
        }
    }
    break;
    case SDLK_TAB:
    {
        append(m_cursor.row() - 1, m_cursor.col() - 1, "    ");
        m_cursor.move(m_cursor.row() + 4, 0);
    }
    break;
    default:
        break;
    }
}

void Selection::startSelection(Cursor& cursor)
{
    if (!hasSelection())
    {
        start_row = end_row = (int)cursor.row();
        start_col = end_col = (int)cursor.col();

        start_y = cursor.y();
    }
}

bool Selection::hasSelection()
{
    return start_row != end_row;
}

void Selection::draw()
{
    int glyph_height = 0, glyph_width;
    TTF_SizeText(Editor::getFont(), "_", &glyph_width, &glyph_height);
    SDL_Rect selection_rect = { (start_row - 1) * glyph_width, start_y, (end_row - start_row) * glyph_width, glyph_height};
    SDL_SetRenderDrawColor(Editor::getRenderer(), 0xff, 0xff, 0xff, 0x40);
    SDL_RenderFillRect(Editor::getRenderer(), &selection_rect);
}
