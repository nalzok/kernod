cmake_minimum_required(VERSION 3.10)
project(kernod C)

set(CMAKE_C_STANDARD 11)

add_executable(kernod main.c)
target_compile_options(kernod PRIVATE -Wall -Wextra -pedantic -c)
target_link_libraries(kernod kcgi z)
