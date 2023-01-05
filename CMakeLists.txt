cmake_minimum_required(VERSION 3.11)
project(it)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_GENERATOR_PLATFORM x64)

# PYBIND11 setup
#add_subdirectory("extern/pybind11")

# SDL2 setup -----------------------------------------
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
    src/buffer.cpp
    src/parser.cpp
    src/main.cpp
    src/text.h
    src/text.cpp
    src/cursor.cpp
    src/platform.cpp
)

add_executable(${PROJECT_NAME} ${IMG_FILES} ${SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${PYTHON_LIBS} Comdlg32) #pybind11::embed)