cmake_minimum_required(VERSION 3.11)
project(it)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#SDL2 setup -----------------------------------------
set(SDL2_ROOT "${CMAKE_CURRENT_LIST_DIR}/lib/SDL2")
set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/SDL2/include")

# Support both 32 and 64 bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(SDL2_LIBRARIES "${SDL2_ROOT}/lib/x64/SDL2.lib;${SDL2_ROOT}/lib/x64/SDL2main.lib;${SDL2_ROOT}/lib/x64/SDL2_ttf.lib;${SDL2_ROOT}/lib/x64/SDL2_mixer.lib")
else ()
    set(SDL2_LIBRARIES "${SDL2_ROOT}/lib/x86/SDL2.lib;${SDL2_ROOT}/lib/SDL2/lib/x86/SDL2main.lib")
endif ()
string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
#----------------------------------------------------

include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

set(SRC
    src/main.cpp)

add_executable(${PROJECT_NAME} ${IMG_FILES} ${SRC})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})