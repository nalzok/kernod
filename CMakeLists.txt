cmake_minimum_required(VERSION 3.10)
project(kernod C)

set(CMAKE_C_STANDARD 11)

add_executable(kernod main.c main.h auth/process_login.c auth/process_login.h index/process_index.c index/process_index.h config.h)
target_compile_options(kernod PRIVATE -Wall -Wextra -pedantic -c)
target_link_libraries(kernod kcgi kcgihtml z)
