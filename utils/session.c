//
// Created by 孙庆耀 on 2018/7/21.
//

#include "session.h"
#include "../config.h"
#include "../utils/utils.h"

#include <ksql.h>
#include <string.h>
#include <inttypes.h>


enum session_stmt {
    STMT_SELECT_SESSION,
    STMT__MAX
};

static const char *const stmts[STMT__MAX] = {
        "SELECT username, email, join_ts FROM users "
        "WHERE user_id = ?",
};

struct session_t session = {0, NULL, NULL, NULL};


extern struct session_t *init_session(struct kreq *req) {

    /* Check if session ID is present in cookie */

    struct kpair *psession_id = req->cookiemap[COOKIE_SESSION_ID];
    if (psession_id == NULL) {
        return NULL;
    }

    /* Get user_id from Redis */

    struct redisReply *reply = get_value("session", psession_id->parsed.s);
    if (reply == NULL || reply->type != REDIS_REPLY_STRING) {
        freeReplyObject(reply);
        return NULL;
    }

    session.user_id = strtoimax(reply->str, NULL, 10);

    freeReplyObject(reply);

    /* Initialize database connection */

    struct ksqlcfg cfg;
    struct ksql *sql;

    ksql_cfg_defaults(&cfg);
    cfg.flags |= KSQL_FOREIGN_KEYS;
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    /* Select data from SQLite */

    struct ksqlstmt *select_session_stmt;
    enum ksqlc select_session_rv;

    ksql_stmt_alloc(sql, &select_session_stmt, NULL, STMT_SELECT_SESSION);
    ksql_bind_int(select_session_stmt, 0, session.user_id);
    select_session_rv = ksql_stmt_step(select_session_stmt);

    if (select_session_rv != KSQL_ROW) {
        flash("You are not logged in.", MSG_TYPE_DANGER);
        ksql_stmt_free(select_session_stmt);
        ksql_free(sql);
        return NULL;
    }

    const char *username_volatile, *email_volatile, *join_ts_volatile;
    ksql_result_str(select_session_stmt, &username_volatile, 0);
    ksql_result_str(select_session_stmt, &email_volatile, 1);
    ksql_result_str(select_session_stmt, &join_ts_volatile, 2);

    session.username = strdup(username_volatile);
    session.email = strdup(email_volatile);
    session.join_ts = strdup(join_ts_volatile);

    ksql_stmt_free(select_session_stmt);
    ksql_free(sql);

    if (session.username == NULL
        || session.email == NULL
        || session.join_ts == NULL) {
        return NULL;
    }

    return &session;
}
