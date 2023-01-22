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

    if (Editor::get() == nullptr)
    {
        SDL_Quit();
        return -1;
    }

    SDL_RenderSetLogicalSize(Editor::getRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);


    Theme theme;
    theme.keyword = 0x689d6a;
    theme.symbols = 0xebdbb2;
    theme.numbers = 0xb16286;
    theme.namepce = 0xfe8019;
    theme.string_ = 0x98971a;

    int cursor_w = 0, cursor_h = 0;
    TTF_SizeText(Editor::getFont(), "_", &cursor_w, &cursor_h);

    size_t begin_offset = 0, end_offset = Editor::getScreenCols() - 1;

    Buffer buffer(filename);

    std::stringstream goto_line;
    bool goto_ = false;

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
                    //goto_line << event.text.text[0];
                }
            }

            if (event.type == SDL_TEXTEDITING)
            {
                //printf("start: %d\n", event.edit.start);
            }

            // buffer
            buffer.update(event);

            if (event.type == SDL_KEYDOWN)
            {

                //if (ctrlKey(SDL_SCANCODE_G))
                //{
                //    goto_ = !goto_;
                //}

                switch(event.key.keysym.sym)
                {
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    {
                        if (goto_)
                        {
                            size_t line_num = 0;
                            goto_line >> line_num;
                            if (line_num > buffer.size())
                                line_num = buffer.size();

                            goto_line.str("");
                            goto_line.clear();
                            goto_ = false;
                           
                            buffer.redraw();
                        }
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
            buffer.draw(begin_offset, end_offset, theme);
        }

        // Status bar
        {
            // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accuracy and out of bounds cursor_row position
            const std::string status_line = "row: " + std::to_string(buffer.getCursorRow()) + " | col: " + std::to_string(buffer.getCursorCol());

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
