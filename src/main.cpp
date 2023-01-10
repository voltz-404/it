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


    int screen_max_cols = Editor::getScreenCols();
    int screen_max_rows = Editor::getScreenRows();

    int begin_offset = 0, end_offset = screen_max_cols - 1;

    Cursor cursor(cursor_w, cursor_h, screen_max_rows, screen_max_cols);

    Buffer buffer(filename);
    std::string status_line = "row: " + std::to_string(1) + " | col: " + std::to_string(1);

    bool done = false;
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
                done = true;


            SDL_StartTextInput();
            if (event.type == SDL_TEXTINPUT)
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
                else
                    buffer.append(cursor.row() - 1, cursor.col() - 1, event.text.text);

                cursor.moveRight(buffer.getLineSize(cursor.col() - 1));
            }
            if (event.type == SDL_TEXTEDITING)
            {
                // printf("%s\n", event.text.text);
            }


            if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
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
                        if (cursor.y() == 0 && cursor.col() > 1)
                            buffer.redraw();
                        cursor.moveUp();
                        break;
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    {
                        buffer.appendNewLine(cursor.col(), cursor.row());
                    }
                    case SDLK_DOWN:
                        if (cursor.col() >= screen_max_cols)
                            buffer.redraw();

                        cursor.moveDown(buffer.size());
                        break;
                    case SDLK_BACKSPACE:
                    {
                        buffer.deleteAt(cursor.row(), cursor.col());

                        if (buffer.size() >= 1 && cursor.row() == 1 && cursor.col() > 1)
                            cursor.moveUp();
                    }
                    case SDLK_LEFT:
                        cursor.moveLeft();
                        break;
                    case SDLK_RIGHT:
                        cursor.moveRight(buffer.getLineSize(cursor.col() - 1));
                        break;
                    case SDLK_TAB: 
                        buffer.append(cursor.row() - 1, cursor.col() - 1, "    ");
                        // TODO: cursor.move(4, 0); move row by 4 instead of the for loop
                        for (int i = 0; i < 4; i++) {
                            cursor.moveRight(buffer.getLineSize(cursor.col() - 1));
                        }
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

        // Status bar
        {

            // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accuracy and out of bounds cursor_row position
            status_line = "row: " + std::to_string(cursor.row()) + " | col: " + std::to_string(cursor.col());

            int status_width = 0;
            TTF_SizeText(Editor::getFont(), status_line.c_str(), &status_width, NULL);

            Text status_text(status_line, SCREEN_WIDTH - status_width, SCREEN_HEIGHT - cursor_h, 0xfb4934);
            status_text.makeTexture();
            status_text.draw();
        }

        // Cursor
        cursor.draw();

        SDL_RenderPresent(Editor::getRenderer());
    }

    // Cleanup
    TTF_CloseFont(Editor::getFont());
    SDL_DestroyRenderer(Editor::getRenderer());
    SDL_DestroyWindow(Editor::getWindow());
    SDL_Quit();

    return 0;
}
