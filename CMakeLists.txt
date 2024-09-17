cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(SimpleNotepad LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Static linking :)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(${PROJECT_NAME} WIN32 
    src/main.c
    src/utils.c
    src/dlgabout.c
    src/exception.c

    res/resources.rc
)

message(STATUS "Fetching zlib...")

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib/
    GIT_TAG        v1.3.1
    GIT_PROGRESS   TRUE
)

set(ZLIB_BUILD_EXAMPLES OFF CACHE INTERNAL "Don't build zlib examples")
set(ZLIB_USE_STATIC_LIBS ON CACHE INTERNAL "Use static zlib libraries")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Don't build shared libraries")

FetchContent_MakeAvailable(zlib)

FetchContent_GetProperties(zlib)
if(NOT zlib_POPULATED)
    FetchContent_Populate(zlib)
endif()

# First attempt.
set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/zlib.lib")

if(EXISTS "${zlib_BINARY_DIR}/zlib.lib")
    message(STATUS "File zlib.lib exists")
    set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/zlib.lib")
else()
    message(STATUS "File zlib.lib does not exist")
    # handle the case where the file does not exist
endif()

file(GLOB_RECURSE ZLIB_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/zlib.lib" "${CMAKE_CURRENT_SOURCE_DIR}/zlibstatic.lib")

if(ZLIB_LIB_PATH)
    message(STATUS "I FUCKING FOUND ZLIB LIB: ${ZLIB_LIB_PATH}")
    set(ZLIB_LIBRARY "${ZLIB_LIB_PATH}")
else()
    message(STATUS "zlib.lib not found!")
    message(STATUS "zlib.lib not found inside the first expected dir. Trying to do smth (fuck this)")
    if(EXISTS "${zlib_BINARY_DIR}/MinSizeRel/zlib.lib")
        message(STATUS "File ${zlib_BINARY_DIR}/MinSizeRel/zlib.lib found in the second expected directory")
        set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/MinSizeRel/zlib.lib")
    else()
        message(WARNING "File zlib.lib does not exist, and seems like it was never existed, but nvm, let's see if the compiler gets the job done.")
        set(ZLIB_LIBRARY "${zlib_BINARY_DIR}/MinSizeRel/zlibstatic.lib")
    endif()
endif()

# FUCKKK YOU MICROSOFT, LIBPNG & ZLIB IDIOT
file(GLOB_RECURSE ZCONF_H_FILES "${CMAKE_CURRENT_SOURCE_DIR}/zconf.h")

if(NOT ZCONF_H_FILES)
    message(FATAL_ERROR "Error: 'zconf.h' not found in the current directory or any subdirectories.")
else()
    foreach(file ${ZCONF_H_FILES})
        message(STATUS "Found zconf.h: ${file}")
        get_filename_component(zconf_header_dir ${file} DIRECTORY)
        message(STATUS "Tried to include zconf.h at ${zconf_header_dir}")
        include_directories(${zconf_header_dir})
    endforeach()
endif()

set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}")

message(STATUS "ZLIB Build dir: ${zlib_BINARY_DIR}")

# need to find the zconf.h
include_directories("${zlib_BINARY_DIR}")
include_directories("${zlib_SOURCE_DIR}")

message(STATUS "Fetching libpng...")

FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/pnggroup/libpng.git
    GIT_TAG        v1.6.44
    GIT_PROGRESS   TRUE
)

set(PNG_TESTS OFF)
set(PNG_SHARED OFF)
set(PNG_TOOLS OFF)
set(PNG_STATIC ON)

FetchContent_MakeAvailable(libpng)

FetchContent_GetProperties(libpng)
if(NOT libpng_POPULATED)
    FetchContent_Populate(libpng)
endif()

set(PNG_LIBRARY "${libpng_BINARY_DIR}/libpng.lib")
set(PNG_INCLUDE_DIR "${libpng_SOURCE_DIR}/include" "${libpng_BINARY_DIR}")

include_directories("${PNG_INCLUDE_DIR}")

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
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${zlib_SOURCE_DIR}
    ${libpng_SOURCE_DIR}
)