cmake_minimum_required(VERSION 3.10)
project(kernod C)

set(CMAKE_C_STANDARD 11)

add_executable(kernod main.c main.h)
target_sources(kernod PRIVATE config.h)
target_sources(kernod PRIVATE index/process_index.c index/process_index.h)
target_sources(kernod PRIVATE auth/process_login.c auth/process_login.h)
target_sources(kernod PRIVATE auth/process_register.c auth/process_register.h)
target_sources(kernod PRIVATE utils/flash.c utils/flash.h)

target_compile_options(kernod PRIVATE -Wall -Wextra -pedantic -c)
target_link_libraries(kernod kcgi kcgihtml ksql sqlite3 sodium z)
