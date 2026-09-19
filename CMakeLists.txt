cmake_minimum_required(VERSION 3.22.1)

project(cleo64 LANGUAGES CXX)

add_library(cleo64 SHARED
    src/main.cpp
)

target_compile_features(cleo64 PRIVATE cxx_std_17)

target_compile_options(cleo64 PRIVATE
    -Wall
    -Wextra
    -fvisibility=hidden
)

target_link_libraries(cleo64
    android
    log
)

set_target_properties(cleo64 PROPERTIES
    OUTPUT_NAME "cleo64"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out"
)
