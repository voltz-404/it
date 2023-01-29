#pragma once

// STDL
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <ctype.h>

#include <stdio.h>

// Windows
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include <SDL_syswm.h>
#endif

// SDL2
#include <SDL.h>
#include <SDL_ttf.h>