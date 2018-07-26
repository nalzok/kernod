//
// Created by 孙庆耀 on 2018/7/21.
//

#include "session.h"
#include "../config.h"
#include "../utils/utils.h"

#include <ksql.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>


enum session_stmt {
    STMT_SELECT_USER_DATA,
    STMT__MAX
};

static const char *const stmts[STMT__MAX] = {
        "SELECT username, email, join_ts FROM users "
        "WHERE user_id = ?",
};

struct session_t session = {NULL};


extern enum populate_session_state populate_session(struct kreq *req) {

    /* Check if session ID is present in cookie */

    struct kpair *psession_id = req->cookiemap[COOKIE_SESSION_ID];
    if (psession_id == NULL) {
        return POPULATE_SESSION_FAILURE;
    }

    /* Allocate memory */

    session.user = calloc(1, sizeof *(session.user));
    if (session.user == NULL) {
        free_session();
        return POPULATE_SESSION_FAILURE;
    }

    /* Get user_id from Redis */

    struct redisReply *reply = get_value("session", psession_id->parsed.s);
    if (reply == NULL || reply->type != REDIS_REPLY_STRING) {
        freeReplyObject(reply);
        free_session();
        return POPULATE_SESSION_FAILURE;
    }

    session.user->user_id = strtoimax(reply->str, NULL, 10);

    freeReplyObject(reply);

    /* Initialize database connection */

    struct ksqlcfg cfg;
    ksql_cfg_defaults(&cfg);
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    struct ksql *sql;
    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    /* Select data from SQLite */

    struct ksqlstmt *select_user_data_stmt;
    ksql_stmt_alloc(sql, &select_user_data_stmt, NULL, STMT_SELECT_USER_DATA);
    ksql_bind_int(select_user_data_stmt, 0, session.user->user_id);

    enum ksqlc select_user_data_rv;
    select_user_data_rv = ksql_stmt_step(select_user_data_stmt);

    if (select_user_data_rv != KSQL_ROW) {
        fprintf(stderr, "Cannot find user %"PRId64"\n", session.user->user_id);
        ksql_stmt_free(select_user_data_stmt);
        ksql_free(sql);
        free_session();
        return POPULATE_SESSION_FAILURE;
    }

    const char *username_volatile, *email_volatile, *join_ts_volatile;
    ksql_result_str(select_user_data_stmt, &username_volatile, 0);
    ksql_result_str(select_user_data_stmt, &email_volatile, 1);
    ksql_result_str(select_user_data_stmt, &join_ts_volatile, 2);

    session.user->username = strdup(username_volatile);
    session.user->email = strdup(email_volatile);
    session.user->join_ts = strdup(join_ts_volatile);

    ksql_stmt_free(select_user_data_stmt);
    ksql_free(sql);

    if (session.user->username == NULL
        || session.user->email == NULL
        || session.user->join_ts == NULL) {
        free_session();
        return POPULATE_SESSION_FAILURE;
    }

    return POPULATE_SESSION_SUCCESS;
}

extern void free_session(void) {
    if (session.user != NULL) {
        free((void *) session.user->username);
        free((void *) session.user->email);
        free((void *) session.user->join_ts);
    }
    free(session.user);
    session = (struct session_t) {NULL};
}

extern const struct user_t *current_user(void) {
    return session.user;
}
