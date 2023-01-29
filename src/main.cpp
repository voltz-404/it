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

struct LineBuffer
{
    std::string line;
    bool is_toggle;

    void pop()
    {
        line.pop_back();
    }

    void clear()
    {
        line.clear();
    }

    void append(const std::string& text)
    {
        line.append(text);
    }

    void toggle()
    {
        is_toggle = !is_toggle;
    }

    size_t get()
    {
        return std::stoi(line);
    }
};

//#define DEBUG

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__) && !defined(DEBUG)
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    int argc = __argc;
    char** argv = __argv;
#else
int main(int argc, char* argv[])
{
#endif

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


    Theme theme = { 0 };
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

    LineBuffer line_buff;
    line_buff.is_toggle = false;
    line_buff.line = "";

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

            switch (event.type)
            {
                case SDL_WINDOWEVENT:
                {
                    switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            int screen_width = 0, screen_height = 0;
                            SDL_GetWindowSize(Editor::getWindow(), &screen_width, &screen_height);
                            SDL_RenderSetLogicalSize(Editor::getRenderer(), screen_width, screen_height);
                            //puts("resized");
                        }
                        break;
                        default: break;
                    }
                }
                    break;
                default: break;
            }
            SDL_StartTextInput();
            if (event.type == SDL_TEXTINPUT)
            {
                if (line_buff.is_toggle)
                {
                    line_buff.append(event.text.text);
                    //goto_line << event.text.text[0];
                }
            }

            if (event.type == SDL_TEXTEDITING)
            {
                //printf("start: %d\n", event.edit.start);
            }

            // buffer
            if (!line_buff.is_toggle)
                buffer.update(event);

            if (event.type == SDL_KEYDOWN)
            {

                if (ctrlKey(SDL_SCANCODE_G))
                {
                    line_buff.toggle();
                }

                switch(event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE:
                    {
                        if (line_buff.is_toggle)
                        {
                            line_buff.pop();
                        }
                    };
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    {
                        if (line_buff.is_toggle)
                        {
                            size_t line_num = line_buff.get();
                            line_buff.toggle();
                            line_buff.clear();

                            buffer.setCursorCol(line_num);
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
            
            if (line_buff.is_toggle)
            {
                drawText("goto line: " + line_buff.line, 0, Editor::getScreenHeight() - cursor_h, 0xfb4934);
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
