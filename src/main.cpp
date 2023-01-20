//#include <pybind11/embed.h>

#include "parser.h"
#include "text.h"
#include "cursor.h"
#include "platform.h"
#include "buffer.h"
#include "internal.h"

#undef main
#undef wmain

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720

bool ctrlKey(const SDL_Event& event, SDL_Scancode key)
{
    return Editor::isKeyDown(SDL_SCANCODE_LCTRL) && Editor::isKeyDown(key);
}

bool isKeyDown(SDL_Scancode key)
{
    return SDL_GetKeyboardState(nullptr)[key];
}

bool isKeyDown(const SDL_Event& event, SDL_KeyCode key)
{
    return (event.type == SDL_KEYDOWN && event.key.keysym.sym == key);
}

bool isKeyUp(const SDL_Event& event, SDL_KeyCode key)
{
    return (event.type == SDL_KEYUP && event.key.keysym.sym == key);
}

struct Selection
{
    int start_x;
    int start_y;

    int end_x;
    int end_y;
    
    int start_row;
    int end_row;
    int start_col;
    int end_col; // not implemented

    //bool hotkey_up;
    bool is_selecting;
    bool is_delete;

    void update(const SDL_Event& event, Cursor& cursor, const size_t line_size)
    {
        // If any key is pressed besides Ctrl-right cancels the selection
        if (!(Editor::isKeyDown(SDL_SCANCODE_LSHIFT) && Editor::isKeyDown(SDL_SCANCODE_RIGHT)) && is_selecting)
        {
            if (isKeyDown(SDL_SCANCODE_BACKSPACE))
            {
                is_delete = true;
            }
            is_selecting = false;
        }

        if (Editor::isKeyDown(SDL_SCANCODE_LSHIFT) && Editor::isKeyDown(SDL_SCANCODE_RIGHT))
        {
            if (!is_selecting)
            {
                start_row = end_row = cursor.row() - 1;
                start_col = end_col = cursor.col();

                start_x = cursor.x();
                start_y = cursor.y();

                is_selecting = true;
            }

            if (end_row < line_size)
                end_row += 1;

            end_x = cursor.x();
            end_y = cursor.y();
        }
    }

    void draw(const int cursor_w, const int cursor_h)
    {
        SDL_Rect rect = { start_x, start_y, (end_row - start_row) * cursor_w, cursor_h };
        SDL_SetRenderDrawColor(Editor::getRenderer(), 0xff, 0xff, 0xff, 0x30);
        SDL_RenderFillRect(Editor::getRenderer(), &rect);
    }
};

int main(int argc, char** argv)
{
    std::string filename;
    if (argc > 1)
        filename = argv[1];
    else
        filename = "";

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || TTF_Init() != 0)
    {
        printf("Error: %s\n TTF Error: %s\n", SDL_GetError(), TTF_GetError());
        return -1;
    }

    if (Editor::get() == nullptr)
    {
        SDL_Quit();
        return -1;
    }

    SDL_RenderSetLogicalSize(Editor::getRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);

    // Pybind11
    // pybind11::scoped_interpreter guard{};
    // pybind11::exec(R"(print("Hello world"))");
    // ------------------------------------------------
    //

    Theme theme;
    theme.keyword = 0x689d6a;
    theme.symbols = 0xebdbb2;
    theme.numbers = 0xb16286;
    theme.namepce = 0xfe8019;
    theme.string_ = 0x98971a;

    int cursor_w = 0, cursor_h = 0;
    TTF_SizeText(Editor::getFont(), "_", &cursor_w, &cursor_h);


    size_t screen_max_cols = Editor::getScreenCols();
    size_t screen_max_rows = Editor::getScreenRows();

    size_t begin_offset = 0, end_offset = screen_max_cols - 1;

    Cursor cursor(cursor_w, cursor_h, screen_max_rows, screen_max_cols);

    Buffer buffer(filename);

    std::stringstream goto_line;
    bool goto_ = false;

    Selection select = { 0 };
    select.is_selecting = false;
    select.is_delete = false;

    bool done = false;
    SDL_StartTextInput();
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(Editor::getWindow()))
                done = true;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                if (goto_)
                    goto_ = false;
                else
                    done = true;
            }


            SDL_StartTextInput();
            if (event.type == SDL_TEXTINPUT)
            {
                if (goto_)
                {
                    goto_line << event.text.text[0];
                }
                else
                {
                    if (event.text.text[0] == '{')
                    {
                        buffer.append(cursor.row() - 1, cursor.col() - 1, "{}");
                    }
                    else if (std::string(event.text.text) == "(")
                    {
                        buffer.append(cursor.row() - 1, cursor.col() - 1, "()");
                    }
                    else if (std::string(event.text.text) == "[")
                    {
                        buffer.append(cursor.row() - 1, cursor.col() - 1, "[]");
                    }
                    else if (std::string(event.text.text) == "\"")
                    {
                        buffer.append(cursor.row() - 1, cursor.col() - 1, "\"\"");
                    }
                    else
                        buffer.append(cursor.row() - 1, cursor.col() - 1, event.text.text);

                    cursor.moveRight(buffer.getLineSize(cursor.col() - 1));
                }
            }

            if (event.type == SDL_TEXTEDITING)
            {
                //printf("start: %d\n", event.edit.start);
            }

            if (event.type == SDL_MOUSEWHEEL)
            {
                // Mouse scroll up
                if (event.wheel.y > 0)
                {
                    buffer.redraw();
                    cursor.move(1, cursor.col() - 1);
                }

                // Mouse scroll down
                if (event.wheel.y < 0 && cursor.col() < buffer.size())
                {
                    buffer.redraw();
                    cursor.move(1, cursor.col() + 1);
                }

            }

            if (event.type == SDL_KEYDOWN)
            {
                if (ctrlKey(event, SDL_SCANCODE_G))
                {
                    goto_ = !goto_;
                }
                else if (ctrlKey(event, SDL_SCANCODE_O))
                {
                    std::string temp_filename = getOpenFileName();
                    if (temp_filename.size() > 0)
                    {
                        filename = temp_filename;
                        cursor.move(1, 1);
                        buffer.openFile(filename);
                    }
                }
                else if (ctrlKey(event, SDL_SCANCODE_S))
                {
                    buffer.saveBuffer();
                }
                else if (ctrlKey(event, SDL_SCANCODE_L))
                {
                    cursor.move(buffer.getLineSize(cursor.col() - 1) + 1, 0);
                }
                else if (ctrlKey(event, SDL_SCANCODE_J))
                {
                    cursor.move(1, 0);
                }
                else if (ctrlKey(event, SDL_SCANCODE_LEFTBRACKET))
                {
                    buffer.append(0, cursor.col() - 1, "    ");
                    cursor.move(cursor.row() + 4, 0);
                }
                else if (ctrlKey(event, SDL_SCANCODE_X))
                {
                    buffer.deleteLine(cursor.col());
                    cursor.move(1, 0);
                    if (cursor.col() > buffer.size())
                        cursor.moveUp();
                }
                else if (ctrlKey(event, SDL_SCANCODE_V))
                {
                    std::string clipboard = SDL_GetClipboardText();
                    buffer.append(cursor.row() - 1, cursor.col() - 1, clipboard);
                    cursor.move(cursor.row() + clipboard.size(), 0);
                }

                // Selection
                select.update(event, cursor, buffer.getLineSize(cursor.col() - 1));

                //if (Editor::isKeyDown(SDL_SCANCODE_A))
                //{
                //    puts("A is pressed");
                //}

                switch(event.key.keysym.sym)
                {

                    case SDLK_F4:
                        goto_ = !goto_;
                    break;
                    case SDLK_END:
                        cursor.move(buffer.getLineSize(cursor.col() - 1) + 1, 0);
                    break;
                    case SDLK_HOME:
                        cursor.move(1, 0);
                        break;
                    case SDLK_F3:
                    {
                        filename = getOpenFileName();
                        cursor.move(1, 1);
                        buffer.openFile(filename);
                    }
                        break;
                    case SDLK_F2:
                    {
                        buffer.saveBuffer();
                    }
                        break;
                    case SDLK_UP:
                    {
                        size_t cursor_row_pos = cursor.row();

                        if (cursor.y() == 0 && cursor.col() > 1)
                            buffer.redraw();
                        cursor.moveUp();

                        if (buffer.getLineSize(cursor.col() - 1) > cursor_row_pos)
                            cursor.move(cursor_row_pos, 0);
                        else
                            cursor.move(buffer.getLineSize(cursor.col() - 1) + 1, 0);

                    }
                        break;
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    {
                        if (goto_)
                        {
                            size_t line_num = 0;
                            goto_line >> line_num;
                            if (line_num > buffer.size())
                                line_num = buffer.size();
                            //printf("%d\n", line_num);

                            goto_line.str("");
                            goto_line.clear();
                            goto_ = false;
                           
                            cursor.move(0, line_num);
                            buffer.redraw();
                        }
                        else
                        {
                            buffer.appendNewLine(cursor.col(), cursor.row());
                            cursor.moveDown(buffer.size());
                        }
                    }
                        break;
                    case SDLK_DOWN:
                    {
                        size_t cursor_row_pos = cursor.row();

                        /// TODO: if buffer is scrolled down, this will be true, fix it
                        if (cursor.col() > screen_max_cols - 1)
                            buffer.redraw();

                        cursor.moveDown(buffer.size());

                        if (buffer.getLineSize(cursor.col() - 1) > cursor_row_pos - 1)
                            cursor.move(cursor_row_pos, 0);
                        else
                            cursor.move(buffer.getLineSize(cursor.col() - 1) + 1, 0);
                    }
                    break;
                    case SDLK_BACKSPACE:
                    {
                        if (select.is_delete == false)
                        {
                            // move cursor to end of the above line when delete from row = 1
                            size_t cursor_row = cursor.row();
                            if (cursor.col() > 1)
                                cursor_row = buffer.getLineSize(cursor.col() - 2) + 2;

                            buffer.deleteAt(cursor.row(), cursor.col());

                            if (buffer.size() >= 1 && cursor.row() == 1 && cursor.col() > 1)
                            {
                                cursor.moveUp();
                                cursor.move(cursor_row, 0);
                            }

                            cursor.moveLeft();
                        }
                        else
                        {
                            buffer.deleteAt(select.start_row + 2, select.start_col, (size_t)(select.end_row - select.start_row));
                            cursor.move(select.start_row + 1, 0);
                            select.is_delete = false;
                        }
                    }
                        break;
                    case SDLK_LEFT:
                        cursor.moveLeft();
                        break;
                    case SDLK_RIGHT:
                        cursor.moveRight(buffer.getLineSize(cursor.col() - 1));
                        break;
                    case SDLK_TAB: 
                        buffer.append(cursor.row() - 1, cursor.col() - 1, "    ");
                        cursor.move(cursor.row() + 4, 0);
                        break;
                    default:
                        break;
                }
            }
        }

        // Render
        SDL_SetRenderDrawBlendMode(Editor::getRenderer(), SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(Editor::getRenderer(), 0x1d, 0x20, 0x21, 0xff);
        SDL_RenderClear(Editor::getRenderer());

        // Render the editor
        {
            int cursor_col_offset = cursor.y() > 0 ? cursor.y() / cursor_h + 1 : 1;

            buffer.draw(begin_offset, end_offset, cursor.col(), cursor_col_offset, cursor.y(), screen_max_cols, theme);
        }

        if (select.is_selecting)
        {
            select.draw(cursor_w, cursor_h);
        }

        //if (is_selected)
        //{
        //    SDL_Rect rect = { (start_row - 1) * cursor_w, cursor.y(), cursor_w * selection, cursor_h};
        //    SDL_SetRenderDrawColor(Editor::getRenderer(), 0xff, 0xff, 0xff, 0x30);
        //    SDL_RenderFillRect(Editor::getRenderer(), &rect);
        //}

        // Status bar
        {
            // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accuracy and out of bounds cursor_row position
            const std::string status_line = "row: " + std::to_string(cursor.row()) + " | col: " + std::to_string(cursor.col());

            int status_width = 0;
            TTF_SizeText(Editor::getFont(), status_line.c_str(), &status_width, NULL);

            drawText(status_line, Editor::getScreenWidth() - status_width, Editor::getScreenHeight() - cursor_h, 0xfb4934);
            
            if (goto_)
            {
                drawText("goto line: " + goto_line.str(), 0, SCREEN_HEIGHT - cursor_h, 0xfb4934);
            }
            else
            {
                const std::string file_saved = !buffer.fileSaved() ? "(!)" : "";
                drawText("file: " + buffer.getFilename() + file_saved, 0, Editor::getScreenHeight() - cursor_h, 0xfb4934);
            }
        }

        // Cursor
        cursor.draw();

        SDL_RenderPresent(Editor::getRenderer());
    }

    SDL_StopTextInput();
    // Cleanup
    TTF_CloseFont(Editor::getFont());
    SDL_DestroyRenderer(Editor::getRenderer());
    SDL_DestroyWindow(Editor::getWindow());
    SDL_Quit();

    delete Editor::get();

    return 0;
}
