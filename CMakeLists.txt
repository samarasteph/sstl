cmake_minimum_required(VERSION 3.10)

project(sslt LANGUAGES CXX VERSION 1.0)

include(CTest)
enable_testing()

set(CMAKE_SOURCE_DIR src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)
