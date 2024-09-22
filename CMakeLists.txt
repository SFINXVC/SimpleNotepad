cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(SimpleNotepad LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -v")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Static linking :)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(libs)

add_executable(${PROJECT_NAME} WIN32 
    src/main.c
    src/utils.c
    src/dlgabout.c
    src/exception.c

    res/resources.rc
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/libpng
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    user32.lib 
    Msimg32.lib
    gdiplus.lib 
    comctl32.lib 
    dwmapi.lib 
    shlwapi.lib 
    uxtheme.lib

    zlibstatic
    png_static
    discord-rpc
)