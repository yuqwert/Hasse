cmake_minimum_required(VERSION 3.15)
project(intellihasse C)

set(CMAKE_C_STANDARD 99)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})

set(CMAKE_C_FLAGS "-fprofile-instr-generate -fcoverage-mapping -fsanitize=address -framework GLUT -framework OpenGL")

add_executable(intellihasse hasse.c)
