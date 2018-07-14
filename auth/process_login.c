//
// Created by 孙庆耀 on 2018/7/10.
//

#include "process_login.h"
#include "../config.h"

#include <kcgihtml.h>
#include <ksql.h>
#include <string.h>


enum stmt {
    STMT_SELECT_PASSWORD,
    STMT__MAX
};

enum login {
    LOGIN_SUCCESS,
    LOGIN_INVALID_USERNAME,
    LOGIN_INVALID_PASSWORD,
    LOGIN_WRONG_PASSWORD,
};

static const char *const stmts[STMT__MAX] = {
        "SELECT password FROM users "
        "WHERE username = ?",
};

static enum login validate_login_form(struct kreq *req);

static void render_login_form(struct khtmlreq *htmlreq);


extern enum khttp process_login(struct kreq *req) {

    if (KMETHOD_GET == req->method) {
        khttp_head(req, kresps[KRESP_STATUS],
                   "%s", khttps[KHTTP_200]);
        khttp_head(req, kresps[KRESP_CONTENT_TYPE],
                   "%s", kmimetypes[req->mime]);
        khttp_body(req);

        struct khtmlreq htmlreq;
        khtml_open(&htmlreq, req, KHTML_PRETTY);
        khtml_elem(&htmlreq, KELEM_P);
        khtml_puts(&htmlreq, "Please login\n");
        render_login_form(&htmlreq);
        khtml_closeelem(&htmlreq, 0);
        khtml_close(&htmlreq);

        khttp_free(req);
        return KHTTP_200;

    } else if (KMETHOD_POST == req->method) {
        switch (validate_login_form(req)) {
            default:
                khttp_head(req, kresps[KRESP_LOCATION],
                           "%s", pages[PAGE_INDEX]);
                khttp_body(req);
                khttp_free(req);
                return KHTTP_302;
        }

    } else {
        khttp_head(req, kresps[KRESP_STATUS],
                   "%s", khttps[KHTTP_405]);
        khttp_head(req, kresps[KRESP_CONTENT_TYPE],
                   "%s", kmimetypes[KMIME_TEXT_HTML]);
        khttp_puts(req, khttps[KHTTP_405]);
        khttp_body(req);
        khttp_free(req);
        return KHTTP_405;
    }
}


static void render_login_form(struct khtmlreq *htmlreq) {
    khtml_elem(htmlreq, KELEM_P);
    khtml_attr(htmlreq, KELEM_FORM,
               KATTR_METHOD, "post",
               KATTR_ENCTYPE, "multipart/form-data",
               KATTR_ACTION, "login",
               KATTR__MAX);
    khtml_elem(htmlreq, KELEM_FIELDSET);
    khtml_elem(htmlreq, KELEM_LEGEND);
    khtml_puts(htmlreq, "Login");
    khtml_closeelem(htmlreq, 1);

    khtml_elem(htmlreq, KELEM_P);
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
    khtml_closeelem(htmlreq, 1);

    khtml_elem(htmlreq, KELEM_P);
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
    khtml_closeelem(htmlreq, 1);

    khtml_elem(htmlreq, KELEM_P);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "submit",
               KATTR__MAX);
    khtml_closeelem(htmlreq, 1);
}


static enum login validate_login_form(struct kreq *req) {

    struct ksqlcfg cfg;
    struct ksql *sql;
    size_t stmtsz = sizeof stmts / sizeof stmts[0];

    ksql_cfg_defaults(&cfg);
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = stmtsz;

    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    struct ksqlstmt *stmt;
    ksql_stmt_alloc(sql, &stmt, NULL, STMT_SELECT_PASSWORD);

    struct kpair *pusername;
    pusername = req->fieldmap[KEY_USERNAME];
    if (!pusername) {
        ksql_stmt_free(stmt);
        ksql_free(sql);
        return LOGIN_INVALID_USERNAME;
    }

    ksql_bind_str(stmt, 0, pusername->parsed.s);

    ksql_stmt_step(stmt);

    const char *hashed;
    ksql_result_str(stmt, &hashed, 0);

    ksql_stmt_free(stmt);
    ksql_free(sql);

    struct kpair *ppassword;
    ppassword = req->fieldmap[KEY_PASSWORD];
    if (!ppassword) {
        return LOGIN_INVALID_PASSWORD;
    }

    if (strcmp(ppassword->parsed.s, hashed) == 0) {
        return LOGIN_SUCCESS;
    } else {
        return LOGIN_WRONG_PASSWORD;
    }
}
