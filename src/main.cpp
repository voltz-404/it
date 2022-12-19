#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#undef main
#undef wmain

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720


#define ENUM(x) #x

SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

void drawText(const char* text, int font_size, int x, int y, uint32_t color)
{
    uint8_t red = color >> 16 & 0xff, green = color >> 8 & 0xff, blue = color & 0xff;
    SDL_Color text_color      = { red, green, blue, 0xff }; 
    SDL_Surface* text_surface = nullptr; 
    SDL_Texture* text_texture = nullptr;

    if ((text_surface =  TTF_RenderUTF8_Blended_Wrapped(font, text, text_color, 0)) == nullptr)
    {
        printf("Could not creaetd surface from font: %s\n", TTF_GetError());
        return;
    }
    if ((text_texture = SDL_CreateTextureFromSurface(renderer, text_surface)) == nullptr)
    {
        printf("Could not creaetd texture from surface: %s\n", TTF_GetError());
        SDL_FreeSurface(text_surface);
        return;
    }

    SDL_Rect text_rect       = { x, y, text_surface->w, text_surface->h };
    SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

SDL_Color hexColorToSDLcolor(uint32_t color)
{
    uint8_t red = color >> 16 & 0xff, green = color >> 8 & 0xff, blue = color & 0xff;
    SDL_Color sdl_color = { red, green, blue, 0xff };

    return sdl_color;
}

void drawCursor(int x, int y, int w, int h, uint32_t color)
{
    SDL_Color cursor_color = hexColorToSDLcolor(color);
    SDL_Rect cursor_rect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, cursor_color.r, cursor_color.g, cursor_color.b, 0xff);
    SDL_RenderDrawRect(renderer, &cursor_rect);
}

int main(int, char**)
{
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

    font = TTF_OpenFont("./assets/fonts/liberation-mono.ttf", 18);
    if (font == nullptr)
    {
        printf("%s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    int cursor_row = 1, cursor_col = 1;
    int cursor_x = 0, cursor_y = 0, cursor_w = 0, cursor_h = 0;
    TTF_SizeText(font, "A", &cursor_w, &cursor_h);

    std::vector<std::string> test_text = {"Lorem ipsum dolor sit amet, concectetur adipiscing elit.", "\n", "Praesent elementum semper tellus ac tincidunt.", "Suspendisse sit amet mauris vel justo lacinia gravida"};
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

            if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_UP: 
                        if (cursor_y >= cursor_h)
                        {
                            cursor_row = 1;
                            cursor_x = 0;

                            cursor_col -= 1;
                            cursor_y -= cursor_h;
                        }
                        break;
                    case SDLK_DOWN: 
                        if (cursor_y + cursor_h <= SCREEN_HEIGHT - cursor_h && cursor_col < test_text.size())
                        {
                            cursor_row = 1;
                            cursor_x = 0;

                            cursor_col += 1;
                            cursor_y += cursor_h;
                        }
                        ; 
                        break;
                    case SDLK_LEFT: 
                        if (cursor_x >= cursor_w)
                        {
                            cursor_row -= 1;
                            cursor_x -= cursor_w;
                        } 
                        break;
                    case SDLK_RIGHT: 
                        if (cursor_x + cursor_w < SCREEN_WIDTH - cursor_w && cursor_row + 1 <= test_text[cursor_col - 1].size())
                        { 
                            // printf("this column letter size: %llu\n", test_text[cursor_col - 1].size() - 1);
                            cursor_row += 1;
                            cursor_x += cursor_w; 
                        }
                        break;
                    default:
                        break; 
                }
            }
        }

        // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accurace and out of bounds cursor_row position
        status_line = "row: " + std::to_string(cursor_row) + "/" + std::to_string(test_text[cursor_col - 1].size()) +  " | col: " + std::to_string(cursor_col) + " ch = " + test_text[cursor_col - 1][cursor_row - 1];

        // Render
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0x15, 0x15, 0x15, 0xff);
        SDL_RenderClear(renderer);

        // Draws text to the screen line by line
        for (int i = 0; i < test_text.size(); i++)
        {
            drawText(test_text[i].c_str(), 18, 0.f, i * cursor_h, 0xffffff);
        }

        // int status_w = 0, status_h = 0;
        // TTF_SizeText(font, status_line.c_str(), &status_w, &status_h);
        drawText(status_line.c_str(), 18, SCREEN_WIDTH - 300, SCREEN_HEIGHT - cursor_h, 0x00ff00);

        drawCursor(cursor_x, cursor_y, cursor_w, cursor_h, 0x0000ff);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
