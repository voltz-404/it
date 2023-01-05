//#include <pybind11/embed.h>

#include "parser.h"
#include "text.h"
#include "cursor.h"
#include "platform.h"
#include "buffer.h"

#undef main
#undef wmain

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720


SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

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

    TTF_Init();
    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Blaze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);

    // Setup SDL_Renderer instance
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    font = TTF_OpenFont("./assets/fonts/liberation-mono.ttf", 16);
    if (font == nullptr)
    {
        printf("%s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

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

    int cursor_row = 1, cursor_col = 1;
    int cursor_x = 0, cursor_y = 0, cursor_w = 0, cursor_h = 0;
    TTF_SizeText(font, "A", &cursor_w, &cursor_h);


    int scren_max_cols = SCREEN_HEIGHT / cursor_h;
    int scren_max_rows = SCREEN_WIDTH / cursor_w;

    int begin_offset = 0, end_offset = scren_max_cols - 1;

    Cursor cursor(cursor_w, cursor_h, scren_max_rows, scren_max_cols);

    Buffer buffer(filename);
    std::string status_line = "row: " + std::to_string(cursor_row) + " | col: " + std::to_string(cursor_row);

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                done = true;


            SDL_StartTextInput();
            if (event.type == SDL_TEXTINPUT)
            {
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
                        cursor.moveUp();
                        break;
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                    {
                        buffer.appendNewLine(cursor.col(), cursor.row());
                    }
                    case SDLK_DOWN:
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
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0x1d, 0x20, 0x21, 0xff);
        SDL_RenderClear(renderer);

        // Render the editor
        {
            int cursor_col_offset = cursor.y() > 0 ? cursor.y() / cursor_h + 1 : 1;

            buffer.draw(renderer, font, begin_offset, end_offset, cursor.col(), cursor_col_offset, cursor.y(), scren_max_cols, theme);
        }

        // Status bar
        {

            // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accuracy and out of bounds cursor_row position
            status_line = "row: " + std::to_string(cursor.row()) + " | col: " + std::to_string(cursor.col());

            int status_width = 0;
            TTF_SizeText(font, status_line.c_str(), &status_width, NULL);

            Text status_text(status_line, SCREEN_WIDTH - status_width, SCREEN_HEIGHT - cursor_h, font, 0xfb4934);
            status_text.makeTexture(renderer);
            status_text.draw(renderer);
            cursor.draw(renderer);
        }


        SDL_RenderPresent(renderer);
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
