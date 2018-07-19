cmake_minimum_required(VERSION 3.10)
project(kernod C)

set(CMAKE_C_STANDARD 11)

add_executable(kernod main.c)
target_sources(kernod PRIVATE config.h handlers/handlers.h utils/utils.h)
target_sources(kernod PRIVATE handlers/index/handle_index.c handlers/index/handle_index.h)
target_sources(kernod PRIVATE handlers/auth/handle_login.c handlers/auth/handle_login.h)
target_sources(kernod PRIVATE handlers/auth/handle_register.c handlers/auth/handle_register.h)
target_sources(kernod PRIVATE utils/flash.c utils/flash.h)
target_sources(kernod PRIVATE utils/html_resp.c utils/html_resp.c)
target_sources(kernod PRIVATE utils/hash_password_alloc.c utils/hash_password_alloc.h)

target_compile_options(kernod PRIVATE -Wall -Wextra -Werror -pedantic -Wstrict-prototypes -c)
target_link_libraries(kernod kcgi kcgihtml ksql sqlite3 sodium z)
