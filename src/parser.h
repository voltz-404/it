#pragma once

#include "pch.h"

struct Theme
{
    uint32_t keyword;
    uint32_t symbols;
    uint32_t numbers;
    uint32_t namepce;
    uint32_t string_;
};

struct Token
{
    enum Type
    {
        UKN,
        STR,
        NUM,
        PAR,
        BRA,
        SEM,
        COL,
        DOT,
        SPA,
    };

    Type type;
    std::string str;
};

std::vector<Token> parser(const std::string& line);

void drawLine(const std::string& line, int x, int y, Theme theme);
