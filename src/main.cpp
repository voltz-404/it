#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <pybind11/embed.h>

#undef main
#undef wmain

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720


SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

void drawText(const char* text, int font_size, int x, int y, uint32_t color)
{
    if (std::string(text).size() < 1)
        return;

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
    SDL_RenderDrawLine(renderer, x, y, x, y + h);
    // SDL_RenderDrawRect(renderer, &cursor_rect);
}

enum State
{
    EDITOR,
    OPEN_FILE,
};

class FileExplorer
{
public:
    FileExplorer()
    : m_items()
    , m_selected(0)
    , m_cursor_pos(0)
    {
        setPath("."); 
    }

    void setPath(const std::string& path)
    {
        m_items.clear();
        m_cursor_pos = 0;
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            m_items.emplace_back(entry.path().string());
        }
    }

    std::string getSelectedItem()
    {
        return m_items.at(m_cursor_pos);
    }

    void moveCursorUp()
    {
        if (m_cursor_pos >= 1)
            m_cursor_pos -= 1;
    }
    
    void moveCursorDown()
    {
        if (m_cursor_pos + 1 < m_items.size())
            m_cursor_pos += 1;
    }

    void draw()
    {
        int i = 0; 
        int h = 0;
        TTF_SizeText(font, "A", nullptr, &h);
        
        for (const std::string& item : m_items)
        {
            SDL_Rect bg_rect = { 0, h * m_cursor_pos, 300, h };
            SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0x30, 0x10);
            SDL_RenderFillRect(renderer, &bg_rect);

            drawText(item.c_str(), 18, 0, h * i, 0xffffff);
            i++;
        }

    }
private:
    int m_selected;
    int m_cursor_pos;
    std::vector<std::string> m_items;
};

std::vector<std::string> openFile(const std::string& path)
{
    std::vector<std::string> buffer;  

    std::ifstream file(path);
    std::string line;
    while (getline(file, line))
    {
        buffer.emplace_back(line);
    }

    return buffer;
}

void drawFileExplorer(const std::string& path, int selected)
{
    std::vector<std::string> items;

    {
        int i = 0; 
        int h = 0;
        TTF_SizeText(font, "A", nullptr, &h);
        
        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            items.emplace_back(entry.path().string());

            SDL_Rect bg_rect = { 0, h * selected, 300, h };
            SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0x30, 0x10);
            SDL_RenderFillRect(renderer, &bg_rect);

            drawText(entry.path().string().c_str(), 18, 0, h * i, 0xFF8F40);
            i++;
        }
    }

}

void moveCursorRight(int *cursor_x, int *cursor_row, int cursor_w)
{
    *cursor_x = *cursor_row * cursor_w; 
    *cursor_row += 1;
}

void drawTextField(const std::vector<std::string>& buffer, int start_offset, int end_offset, int cursor_h)
{
    start_offset = start_offset <= 0 ? 0 : start_offset;
    end_offset   = start_offset + end_offset > buffer.size() ? buffer.size() : start_offset + end_offset;
    for (int i = start_offset, j = 0; i < end_offset; i++, j++) 
    {
        drawText(buffer[i].c_str(), 18, 0.f, j * cursor_h, 0xFFFFFF);
    }
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
    pybind11::scoped_interpreter guard{};
    pybind11::exec(R"(print("Hello world"))");
    // ------------------------------------------------

    int cursor_row = 1, cursor_col = 1;
    int cursor_x = 0, cursor_y = 0, cursor_w = 0, cursor_h = 0;
    TTF_SizeText(font, "A", &cursor_w, &cursor_h);

    std::string filename = "./blank.cpp";

    State state = State::EDITOR;
    int selected = 0;
    FileExplorer file_explorer;
    int scren_max_cols = SCREEN_HEIGHT / cursor_h;
    int scren_max_rows = SCREEN_WIDTH / cursor_w;

    std::vector<std::string> buffer = openFile(filename.c_str());
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
                if (buffer.size() == 0)
                {
                    buffer.emplace_back(event.text.text);
                } else {
                    buffer.at(cursor_col - 1).insert(cursor_row - 1, event.text.text); 
                }

                moveCursorRight(&cursor_x, &cursor_row, cursor_w);
            }
            if (event.type == SDL_TEXTEDITING)
            {
                // printf("%s\n", event.text.text);
            }


            if(event.type == SDL_KEYDOWN)
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_F10:
                        state = state == EDITOR ? OPEN_FILE : EDITOR;
                        break;
                    case SDLK_F2:
                    {
                        std::string item = file_explorer.getSelectedItem();
                        std::stringstream file_content;
                        for (const std::string& line : buffer)
                            file_content << line << "\n";


                        std::ofstream file(filename);
                        if (file.is_open())
                        {
                            file << file_content.str();
                            puts("file saved");
                        }
                    }
                        break;
                    case SDLK_UP: 
                        if (state == OPEN_FILE)
                        {
                            file_explorer.moveCursorUp(); 
                            break;
                        }
                        if (cursor_col > scren_max_cols)
                        {
                            cursor_row = 1;
                            cursor_x = 0;

                            cursor_col -= 1;
                        }
                        if (cursor_col > 1 && cursor_col <= scren_max_cols)
                        {
                            cursor_row = 1;
                            cursor_x = 0;

                            cursor_col -= 1;
                            cursor_y -= cursor_h;
                        }
                        break;
                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:
                        if (state == OPEN_FILE)
                        {
                            std::string item = file_explorer.getSelectedItem();
                            if (std::filesystem::is_directory(std::filesystem::path(item)))
                            {
                                file_explorer.setPath(item);
                            } else
                            {
                                cursor_row = 1;
                                cursor_x = 0;

                                cursor_col = 1;
                                cursor_y = 0;

                                buffer = openFile(item);
                                filename = item;
                                state = EDITOR;
                            }

                            // printf("%s\n", std::filesystem::is_directory(std::filesystem::path(file_explorer.getSelectedItem())));
                            break;
                        }
                        if (buffer.size() >= 1 && buffer[cursor_col - 1][cursor_row - 1] != '\n')
                        {
                            std::string new_line = buffer.at(cursor_col - 1).substr(cursor_row - 1);
                            buffer.at(cursor_col - 1).insert(cursor_row - 1, "\n"); 
                            // delete the rest of the line after the current cursor_row position
                            buffer.at(cursor_col - 1).erase(cursor_row - 1);

                            // add a new line to the buffer with the rest of the line that was remove from last buffer line
                            buffer.insert(buffer.begin() + (cursor_col), new_line);
                        } else
                        {
                            buffer.insert(buffer.begin() + cursor_col, "\n");
                        }
                    case SDLK_DOWN: 
                        if (state == OPEN_FILE)
                        {
                            file_explorer.moveCursorDown();
                            break;
                        }; 
                        if (cursor_col < buffer.size())
                        {
                            cursor_row = 1;
                            cursor_x = 0;

                            cursor_col += 1;
                            if (cursor_y + cursor_h < SCREEN_HEIGHT - cursor_h)
                                cursor_y += cursor_h;
                        }
                        break;
                    case SDLK_BACKSPACE:
                        if (buffer.size() >= 1 && cursor_row > 1)
                        {
                            buffer.at(cursor_col - 1).erase(cursor_row - 2, 1);
                        } else if (cursor_col > 1)
                        {
                            // buffer.at(cursor_col - 2).append("hello"); 
                            buffer.erase(buffer.begin() + (cursor_col - 1));
                            cursor_col -= 1;
                            cursor_y -= cursor_h;
                        }
                    case SDLK_LEFT: 
                        if (cursor_x >= cursor_w)
                        {
                            cursor_row -= 1;
                            cursor_x -= cursor_w;
                        } 
                        break;
                    case SDLK_RIGHT: 
                        if (cursor_x + cursor_w < SCREEN_WIDTH - cursor_w && cursor_row <= buffer[cursor_col - 1].size())
                        { 
                            // printf("this column letter size: %llu\n", buffer[cursor_col - 1].size() - 1);
                            moveCursorRight(&cursor_x, &cursor_row, cursor_w);
                            // cursor_x = cursor_row * cursor_w; 
                            // cursor_row += 1;
                        }
                        break;
                    case SDLK_TAB: 
                        buffer.at(cursor_col - 1).insert(cursor_row - 1, "    ");
                        for (int i = 0; i < 4; i++) {
                            moveCursorRight(&cursor_x, &cursor_row, cursor_w);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accuracy and out of bounds cursor_row position
        status_line = "row: " + std::to_string(cursor_row) + " | col: " + std::to_string(cursor_col) + "/" + std::to_string(scren_max_cols);

        // Render
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0x15, 0x15, 0x15, 0xff);
        SDL_RenderClear(renderer);

        switch (state)
        {
            case State::EDITOR:
            {
                // Draws text to the screen line by line
                drawTextField(buffer, cursor_col - scren_max_cols, scren_max_cols, cursor_h);
                // int status_w = 0, status_h = 0;
                // TTF_SizeText(font, status_line.c_str(), &status_w, &status_h);
                int status_width = 0;
                TTF_SizeText(font, status_line.c_str(), &status_width, NULL);
                drawText(status_line.c_str(), 18, SCREEN_WIDTH - status_width, SCREEN_HEIGHT - cursor_h, 0x00ff00);
                drawCursor(cursor_x, cursor_y, cursor_w, cursor_h, 0xFF8F40);
                break;
            }
            case State::OPEN_FILE:
            {
                // drawFileExplorer("./", selected);
                file_explorer.draw();
                break;
            }
            default: break;
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
