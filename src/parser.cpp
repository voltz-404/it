#include "parser.h"

std::vector<Token> parser(const std::string& line)
{
    std::vector<Token> tokens;

    for (int i = 0; i < line.size(); i++)
    {
        char chr = 0;
        Token::Type type = Token::Type::UKN;
        std::string token = "";
        if (isalpha(line[i]))
        {
            while(isalpha((chr = line[i])))
            {
                token += chr;
                type = Token::Type::STR;
                i++;
            }
            i--;
        } else if (isdigit(line[i]))
        {
            while(isdigit((chr = line[i])))
            {
                token += chr;
                type = Token::Type::NUM;
                i++;
            }
            i--;
        } else {
            if (line[i] == '\n') continue;

            if (line[i] == ' ')
            {
                while(line[i] == ' ')
                {
                    token += line[i];
                    i++;
                }
                i--;
                type = Token::Type::SPA;
            }
            else
            {
                token += line[i];
                type = Token::Type::UKN;
            }
        }
        tokens.push_back({ type, token });
    }

    return tokens;
}

