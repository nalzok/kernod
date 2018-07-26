cmake_minimum_required(VERSION 3.10)
project(kernod C)

set(CMAKE_C_STANDARD 11)

add_executable(kernod
        main.c
        config.h
        handlers/handlers.h
        utils/utils.h)

add_library(handlers STATIC
        handlers/auth/handle_login.c handlers/auth/handle_login.h
        handlers/auth/handle_register.c handlers/auth/handle_register.h
        handlers/index/handle_index.c handlers/index/handle_index.h
        handlers/post/handle_posts.c handlers/post/handle_posts.h)

add_library(utils STATIC
        utils/crypto.c utils/crypto.h
        utils/flash.c utils/flash.h
        utils/resp.c utils/resp.c
        utils/session.c utils/session.h
        utils/storage.c utils/storage.h)

target_link_libraries(kernod utils handlers)
target_link_libraries(kernod kcgi kcgihtml ksql sqlite3 sodium hiredis z)

target_compile_options(kernod PRIVATE -Wall -Wextra -Werror -pedantic -Wstrict-prototypes -c)
