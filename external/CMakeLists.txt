cmake_minimum_required(VERSION 3.20)

set(LHE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LiquidHookEx/LiquidHookEx)

add_library(liquid_hook_ex STATIC
    ${LHE_DIR}/Source/LiquidHookEx/Globals.cpp
    ${LHE_DIR}/Source/LiquidHookEx/Pattern.cpp
    ${LHE_DIR}/Source/LiquidHookEx/Process.cpp
    ${LHE_DIR}/Source/LiquidHookEx/SysCallManager.cpp
    ${LHE_DIR}/Include/LiquidHookEx/Syscalls_x64.asm
)

target_include_directories(liquid_hook_ex PUBLIC
    ${LHE_DIR}/Include
)

target_compile_definitions(liquid_hook_ex PUBLIC
    LIQUID_HOOK_EX_SYSCALL
)

# MASM support
set_source_files_properties(
    ${LHE_DIR}/Include/LiquidHookEx/Syscalls_x64.asm
    PROPERTIES
    LANGUAGE ASM_MASM
)
