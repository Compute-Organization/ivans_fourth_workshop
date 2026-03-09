cmake_minimum_required(VERSION 3.16)

project(pdua_compiler LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(pdua_compiler
    src/main.cpp
    src/pdua_compiler.cpp
    src/pdua_file_processor.cpp
    src/pdua_instruction.cpp
)
target_include_directories(pdua_compiler PRIVATE include)
target_compile_options(pdua_compiler PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(pdua_simulator
    src/simulator_main.cpp
    src/pdua_simulator.cpp
    src/pdua_file_processor.cpp
    src/pdua_instruction.cpp
)
target_include_directories(pdua_simulator PRIVATE include)
target_compile_options(pdua_simulator PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(workshop_reference
    src/c_programs_main.c
    src/c_programs.c
)
target_include_directories(workshop_reference PRIVATE include)
target_compile_options(workshop_reference PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)