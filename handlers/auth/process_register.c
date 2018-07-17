//
// Created by 孙庆耀 on 2018/7/14.
//

#include "process_register.h"

enum register_state {
    REG_SUCCESS,
    REG_INVALID_USERNAME,
    REG_INVALID_EMAIL,
    REG_INVALID_PASSWORD,
    REG_PASSWORD_MISMATCH,
    REG_USER_ALREADY_EXISTS,

};

enum register_stmt {
    STMT_CREATE_TABLE,
    STMT_INSERT_USER,
    STMT_SELECT_USER,
    STMT__MAX
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

extern enum khttp process_register(struct kreq *req) {
    (void) stmts;
    (void) req;
    return KHTTP_200;
}
