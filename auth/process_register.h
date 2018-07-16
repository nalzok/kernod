//
// Created by 孙庆耀 on 2018/7/14.
//

#ifndef KERNOD_PROCESS_REGISTER_H
#define KERNOD_PROCESS_REGISTER_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>


enum register_stmt {
    STMT_CREATE_TABLE,
    STMT_INSERT_USER,
    STMT_SELECT_USER,
    STMT__MAX
};

enum register_state {
    REG_SUCCESS,
};

static const char *const stmts[STMT__MAX] = {
        "CREATE TABLE IF NOT EXISTS users("
        "  user_id INTEGER PRIMARY KEY,"
        "  username TEXT NOT NULL UNIQUE,"
        "  password TEXT NOT NULL,"
        "  join_ts TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ")",
        "INSERT INTO users(username, password) VALUES(?, ?)",
        "SELECT username, password FROM users",
};

extern enum khttp process_register(struct kreq *req);

#endif //KERNOD_PROCESS_REGISTER_H
