//
// Created by 孙庆耀 on 2018/7/10.
//

#include "handle_login.h"
#include "../../utils/flash.h"
#include "../../utils/http_body_open.h"
#include "../../utils/html_init.h"
#include "../../config.h"

#include <ksql.h>
#include <sodium.h>
#include <string.h>


enum login_state {
    LOGIN_SUCCESS,
    LOGIN_FAILURE,
};

enum login_stmt {
    STMT_SELECT_PASSWORD,
    STMT__MAX
};

static const char *const stmts[STMT__MAX] = {
        "SELECT password FROM users "
        "WHERE username = ?",
};

static void render_login_form(struct khtmlreq *htmlreq);

static enum login_state validate_login_form(struct kreq *req);


extern enum khttp handle_login(struct kreq *req) {
    switch (req->method) {

        case KMETHOD_GET:
            http_body_open(req, KHTTP_200);

            struct khtmlreq htmlreq;
            khtml_open(&htmlreq, req, KHTML_PRETTY);
            html_init(&htmlreq);

            khtml_elem(&htmlreq, KELEM_H1);
            khtml_puts(&htmlreq, "Please login");
            khtml_closeelem(&htmlreq, 1);
            render_login_form(&htmlreq);

            khtml_closeelem(&htmlreq, 0);
            khtml_close(&htmlreq);

            khttp_free(req);
            return KHTTP_200;

        case KMETHOD_POST:
            if (LOGIN_SUCCESS == validate_login_form(req)) {
                khttp_head(req, kresps[KRESP_LOCATION],
                           "%s", pages[PAGE_INDEX]);
            } else {
                khttp_head(req, kresps[KRESP_LOCATION],
                           "%s", pages[PAGE_LOGIN]);
            }
            khttp_body(req);
            khttp_free(req);
            return KHTTP_302;

        default:
            http_body_open(req, KHTTP_405);
            khttp_puts(req, khttps[KHTTP_405]);
            khttp_free(req);
            return KHTTP_405;
    }
}


static void render_login_form(struct khtmlreq *htmlreq) {
    size_t pos = khtml_elemat(htmlreq);

    khtml_attr(htmlreq, KELEM_FORM,
               KATTR_METHOD, "post",
               KATTR_ACTION, "login",
               KATTR__MAX);
    khtml_elem(htmlreq, KELEM_FIELDSET);
    khtml_elem(htmlreq, KELEM_LEGEND);
    khtml_puts(htmlreq, "Login");
    khtml_closeelem(htmlreq, 1);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "login-username",
               KATTR__MAX);
    khtml_puts(htmlreq, "Username");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "text",
               KATTR_ID, "login-username",
               KATTR_NAME, keys[KEY_USERNAME].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "login-password",
               KATTR__MAX);
    khtml_puts(htmlreq, "Password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "login-password",
               KATTR_NAME, keys[KEY_PASSWORD].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "submit",
               KATTR_VALUE, "Submit",
               KATTR__MAX);

    /* this is necessary due to a bug of kcgihtml */
    if (pos != khtml_elemat(htmlreq)) {
        khtml_closeto(htmlreq, pos);
    }
}


static enum login_state validate_login_form(struct kreq *req) {
    struct ksqlcfg cfg;
    struct ksql *sql;

    ksql_cfg_defaults(&cfg);
    cfg.flags |= KSQL_FOREIGN_KEYS;
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    struct ksqlstmt *stmt;
    ksql_stmt_alloc(sql, &stmt, NULL, STMT_SELECT_PASSWORD);

    struct kpair *pusername;
    if (NULL != (pusername = req->fieldmap[KEY_USERNAME])) {
        ksql_bind_str(stmt, 0, pusername->parsed.s);
    } else if (req->fieldnmap[KEY_USERNAME]) {
        flash("Invalid username", MSG_TYPE_DANGER);
        goto out;
    } else {
        flash("Username not provided", MSG_TYPE_DANGER);
        goto out;
    }

    if (KSQL_ROW != ksql_stmt_step(stmt)) {
        flash("User doesn't exist", MSG_TYPE_DANGER);
        goto out;
    }

    const char *hashed_volatile;
    char *hashed;
    ksql_result_str(stmt, &hashed_volatile, 0);
    if (NULL == (hashed = strdup(hashed_volatile))) {
        ksql_stmt_free(stmt);
        ksql_free(sql);
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    ksql_stmt_free(stmt);
    ksql_free(sql);

    struct kpair *ppassword;
    if (NULL != (ppassword = req->fieldmap[KEY_PASSWORD])) {
        if (-1 == sodium_init()) {
            flash("libsodium error", MSG_TYPE_DANGER);
            return LOGIN_FAILURE;
        }
        if (0 == crypto_pwhash_str_verify(hashed, ppassword->parsed.s,
                                          strlen(ppassword->parsed.s))) {
            flash("Welcome!", MSG_TYPE_SUCCESS);
            return LOGIN_SUCCESS;
        } else {
            flash("Wrong password", MSG_TYPE_DANGER);
            return LOGIN_FAILURE;
        }
    } else if (req->fieldnmap[KEY_PASSWORD]) {
        flash("Invalid password", MSG_TYPE_DANGER);
        return LOGIN_FAILURE;
    } else {
        flash("Password not provided", MSG_TYPE_DANGER);
        return LOGIN_FAILURE;
    }

    out:
    ksql_stmt_free(stmt);
    ksql_free(sql);
    return LOGIN_FAILURE;
}
