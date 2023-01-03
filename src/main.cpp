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

//#include <pybind11/embed.h>

#include "parser.h"
#include "text.h"
#include "cursor.h"

#undef main
#undef wmain

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 720


SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
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

            Text item_text(item.c_str(), 0, h * i, font, 0xffffff);
            item_text.makeTexture(renderer);
            item_text.draw(renderer);
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

void moveCursorRight(int *cursor_x, int *cursor_row, int cursor_w)
{
    *cursor_x = *cursor_row * cursor_w; 
    *cursor_row += 1;
}

void drawTextField(const std::vector<std::string>& buffer, int start_offset, int end_offset, int cursor_h, Theme theme)
{
    start_offset = start_offset <= 0 ? 0 : start_offset;
    end_offset   = start_offset + end_offset > (int)buffer.size() ? (int)buffer.size() : start_offset + end_offset;
    for (int i = start_offset, j = 0; i < end_offset; i++, j++) 
    {
        drawLine(buffer[i], 0, j * cursor_h, theme);
    }
}


void drawLine(const std::string& line, int x, int y, Theme theme)
{
    if (line.size() < 1) return;
    std::vector<Token> tokens = parser(line);

    std::vector<std::string> keywords = { "int", "string", "return", "void", "char", "uint_32", "if", "else", "while", "for" ,"switch", "include", "const", "def", "import"};

    int line_w = 0, line_h = 0;
    TTF_SizeText(font, line.c_str(), &line_w, &line_h);

    line_w += 2;

    Text text(font, x, y);
    text.reserveSurface(line_w, line_h);

    bool string_start = false;
    for (const Token& token : tokens)
    {
        uint32_t color;
        int width = 0, height = 0;
        TTF_SizeText(font, token.str.c_str(), &width, &height);

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

        text.append(token.str.c_str(), color);

    }
    text.makeTexture(renderer);
    text.draw(renderer);
}

int main(int argc, char** argv)
{
    std::string filename;
    if (argc > 1)
        filename = argv[1];
    else
        filename = "./blank.cpp";

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


    State state = State::EDITOR;
    int selected = 0;
    FileExplorer file_explorer;
    int scren_max_cols = SCREEN_HEIGHT / cursor_h;
    int scren_max_rows = SCREEN_WIDTH / cursor_w;

    int begin_offset = 0, end_offset = scren_max_cols - 1;

    Cursor cursor(cursor_w, cursor_h, scren_max_rows, scren_max_cols);

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
                    buffer.at(cursor.col() - 1).insert(cursor.row() - 1, event.text.text);
                }

                cursor.moveRight(buffer[cursor.col() - 1].size());
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
                        cursor.moveUp();

                        if (state == OPEN_FILE)
                        {
                            file_explorer.moveCursorUp(); 
                            break;
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
                                cursor.move(1, 1);

                                buffer = openFile(item);
                                filename = item;
                                state = EDITOR;
                            }

                            break;
                        }
                        if (buffer.size() >= 1 && buffer[cursor.col() - 1][cursor.row() - 1] != '\n')
                        {
                            std::string new_line = buffer.at(cursor.col() - 1).substr(cursor.row() - 1);
                            buffer.at(cursor.col() - 1).insert(cursor.row() - 1, "\n");
                            buffer.at(cursor.col() - 1).erase(cursor.row() - 1);

                            buffer.insert(buffer.begin() + (cursor.col()), new_line);
                        } else
                        {
                            buffer.insert(buffer.begin() + cursor.col(), "\n");
                        }
                    case SDLK_DOWN:
                        cursor.moveDown(buffer.size());
                        
                        if (state == OPEN_FILE)
                        {
                            file_explorer.moveCursorDown();
                            break;
                        }; 
                        break;
                    case SDLK_BACKSPACE:
                        if (buffer.size() >= 1 && cursor.row() > 1)
                        {
                            buffer.at(cursor.col() - 1).erase(cursor.row() - 2, 1);
                        } else if (cursor.col() > 1)
                        {
                            buffer.erase(buffer.begin() + (cursor.col() - 1));
                            cursor.moveUp();
                            cursor_col -= 1;
                            cursor_y -= cursor_h;
                        }
                    case SDLK_LEFT:
                        cursor.moveLeft();
                        break;
                    case SDLK_RIGHT:
                        cursor.moveRight(buffer[cursor.col() - 1].size());
                        break;
                    case SDLK_TAB: 
                        buffer.at(cursor_col - 1).insert(cursor.row() - 1, "    ");
                        // TODO: cursor.move(4, 0); move row by 4 instead of the for loop
                        for (int i = 0; i < 4; i++) {
                            cursor.moveRight(buffer[cursor.col() - 1].size());
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // TODO: unsigned char takes 2 bytes space, messing up with cursor_row accuracy and out of bounds cursor_row position
        status_line = "row: " + std::to_string(cursor.row()) + " | col: " + std::to_string(cursor.col());

        // Render
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0x1d, 0x20, 0x21, 0xff);
        SDL_RenderClear(renderer);

        switch (state)
        {
            case State::EDITOR:
            {
                int cursor_col_offset = cursor.y() > 0 ? cursor.y() / cursor_h + 1 : 1;

                if (cursor.y() == 0 && cursor.col() > 0)
                {
                    begin_offset = cursor.col() - 1;
                    end_offset = cursor.col() - 1 + scren_max_cols;
                }
                if (cursor_col_offset == scren_max_cols && cursor.col() > scren_max_cols - 1 )
                {
                    //drawTextField(buffer, cursor.col() - scren_max_cols, scren_max_cols, cursor_h, theme);
                    begin_offset = cursor.col() - scren_max_cols;
                    end_offset = cursor.col() - 1;
                }

                for (int i = begin_offset, j = 0; i < buffer.size(); i++, j++)
                {
                    drawLine(buffer[i], 0, j * cursor_h, theme);
                }
                
                int status_width = 0;
                TTF_SizeText(font, status_line.c_str(), &status_width, NULL);

                Text status_text(status_line, SCREEN_WIDTH - status_width, SCREEN_HEIGHT - cursor_h, font, 0xfb4934);
                status_text.makeTexture(renderer);
                status_text.draw(renderer);
                cursor.draw(renderer);
                //drawCursor(cursor_x, cursor_y, cursor_w, cursor_h, 0xFF8F40);
                break;
            }
            case State::OPEN_FILE:
            {
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
