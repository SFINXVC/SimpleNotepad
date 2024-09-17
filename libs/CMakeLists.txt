cmake_minimum_required(VERSION 3.15)

add_subdirectory(zlib)

set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
set(ZLIB_LIBRARY ${CMAKE_BINARY_DIR}/libs/zlib/zlib.lib)

include_directories(${CMAKE_BINARY_DIR}/libs/zlib)

add_subdirectory(libpng)