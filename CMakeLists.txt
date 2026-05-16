cmake_minimum_required(VERSION 3.20)
project(sir_xternal LANGUAGES CXX ASM_MASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# LiquidHookEx as static library
add_subdirectory(external)

# Our project
add_subdirectory(src/app)
