//
// Created by 孙庆耀 on 2018/7/14.
//

#include "handle_register.h"
#include "../../utils/flash.h"
#include "../../utils/http_body_open.h"
#include "../../utils/html_init.h"
#include "../../config.h"

#include <ksql.h>
#include <sodium.h>
#include <string.h>


enum register_state {
    REG_SUCCESS,
    REG_FAILURE
};

enum register_stmt {
    STMT_SELECT_USER,
    STMT_SELECT_EMAIL,
    STMT_INSERT_USER,
    STMT__MAX
};

static const char *const stmts[STMT__MAX] = {
        "SELECT * FROM users "
        "WHERE username = ?",
        "SELECT * FROM users "
        "WHERE email = ?",
        "INSERT INTO users(username, email, password) "
        "VALUES(?, ?, ?)",
};

static void render_register_form(struct khtmlreq *htmlreq);

static enum register_state validate_register_form(struct kreq *req);

static char *hash_password_alloc(const char *password);


extern enum khttp handle_register(struct kreq *req) {
    switch (req->method) {

        case KMETHOD_GET:
            http_body_open(req, KHTTP_200);

            struct khtmlreq htmlreq;
            khtml_open(&htmlreq, req, KHTML_PRETTY);
            html_init(&htmlreq);

            khtml_elem(&htmlreq, KELEM_H1);
            khtml_puts(&htmlreq, "Join now!");
            khtml_closeelem(&htmlreq, 1);
            render_register_form(&htmlreq);

            khtml_closeelem(&htmlreq, 0);
            khtml_close(&htmlreq);

            khttp_free(req);
            return KHTTP_200;

        case KMETHOD_POST:
            if (REG_SUCCESS == validate_register_form(req)) {
                khttp_head(req, kresps[KRESP_LOCATION],
                           "%s", pages[PAGE_LOGIN]);
            } else {
                khttp_head(req, kresps[KRESP_LOCATION],
                           "%s", pages[PAGE_REGISTER]);
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

static void render_register_form(struct khtmlreq *htmlreq) {
    size_t pos = khtml_elemat(htmlreq);

    khtml_attr(htmlreq, KELEM_FORM,
               KATTR_METHOD, "post",
               KATTR_ACTION, "register",
               KATTR__MAX);
    khtml_elem(htmlreq, KELEM_FIELDSET);
    khtml_elem(htmlreq, KELEM_LEGEND);
    khtml_puts(htmlreq, "Register");
    khtml_closeelem(htmlreq, 1);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "register-username",
               KATTR__MAX);
    khtml_puts(htmlreq, "Username");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "text",
               KATTR_ID, "register-username",
               KATTR_NAME, keys[KEY_USERNAME].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "register-email",
               KATTR__MAX);
    khtml_puts(htmlreq, "Email");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "text",
               KATTR_ID, "register-email",
               KATTR_NAME, keys[KEY_EMAIL].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "register-password",
               KATTR__MAX);
    khtml_puts(htmlreq, "Password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "register-password",
               KATTR_NAME, keys[KEY_PASSWORD].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "register-password2",
               KATTR__MAX);
    khtml_puts(htmlreq, "Retype password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "register-password2",
               KATTR_NAME, keys[KEY_PASSWORD2].name,
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

static enum register_state validate_register_form(struct kreq *req) {
    struct ksqlcfg cfg;
    struct ksql *sql;

    ksql_cfg_defaults(&cfg);
    cfg.flags |= KSQL_FOREIGN_KEYS;
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    struct ksqlstmt *select_user_stmt;
    struct ksqlstmt *select_email_stmt;
    struct ksqlstmt *insert_stmt;
    ksql_stmt_alloc(sql, &select_user_stmt, NULL, STMT_SELECT_USER);
    ksql_stmt_alloc(sql, &select_email_stmt, NULL, STMT_SELECT_EMAIL);
    ksql_stmt_alloc(sql, &insert_stmt, NULL, STMT_INSERT_USER);

    struct kpair *pusername;
    if (NULL != (pusername = req->fieldmap[KEY_USERNAME])) {
        ksql_bind_str(select_user_stmt, 0, pusername->parsed.s);
        ksql_bind_str(insert_stmt, 0, pusername->parsed.s);
    } else if (req->fieldnmap[KEY_USERNAME]) {
        flash("Invalid username", MSG_TYPE_DANGER);
        goto out;
    } else {
        flash("Username not provided", MSG_TYPE_DANGER);
        goto out;
    }

    struct kpair *pemail;
    if (NULL != (pemail = req->fieldmap[KEY_EMAIL])) {
        ksql_bind_str(select_email_stmt, 0, pemail->parsed.s);
        ksql_bind_str(insert_stmt, 1, pemail->parsed.s);
    } else if (req->fieldnmap[KEY_EMAIL]) {
        flash("Invalid email", MSG_TYPE_DANGER);
        goto out;
    } else {
        flash("Email not provided", MSG_TYPE_DANGER);
        goto out;
    }

    struct kpair *ppassword, *ppassword2;
    ppassword = req->fieldmap[KEY_PASSWORD];
    ppassword2 = req->fieldmap[KEY_PASSWORD2];
    if (NULL == ppassword || NULL == ppassword2) {
        flash("Invalid password", MSG_TYPE_DANGER);
        goto out;
    }

    if (0 == strcmp(ppassword->parsed.s,
                    ppassword2->parsed.s)) {
        char *hashed = hash_password_alloc(ppassword->parsed.s);
        if (NULL != hashed) {
            ksql_bind_str(insert_stmt, 2, hashed);
            free(hashed);
        } else {
            flash("Error hashing password", MSG_TYPE_DANGER);
            goto out;
        }
    } else {
        flash("Passwords mismatch", MSG_TYPE_DANGER);
        goto out;
    }

    enum ksqlc select_user_rv, select_email_rv;
    select_user_rv = ksql_stmt_step(select_user_stmt);
    select_email_rv = ksql_stmt_step(select_email_stmt);
    ksql_stmt_free(select_user_stmt);
    ksql_stmt_free(select_email_stmt);
    if (KSQL_ROW == select_user_rv) {
        flash("Username already taken", MSG_TYPE_DANGER);
        goto out;
    } else if (KSQL_DONE != select_user_rv) {
        flash("Database error", MSG_TYPE_DANGER);
        goto out;
    }
    if (KSQL_ROW == select_email_rv) {
        flash("Email already taken", MSG_TYPE_DANGER);
        goto out;
    } else if (KSQL_DONE != select_email_rv) {
        flash("Database error", MSG_TYPE_DANGER);
        goto out;
    }

    if (KSQL_DONE == ksql_stmt_step(insert_stmt)) {
        flash("User created", MSG_TYPE_SUCCESS);
    } else {
        flash("Database error", MSG_TYPE_DANGER);
        goto out;
    }

    ksql_stmt_free(insert_stmt);
    ksql_free(sql);
    return REG_SUCCESS;

    out:
    ksql_stmt_free(insert_stmt);
    ksql_free(sql);
    return REG_FAILURE;
}

static char *hash_password_alloc(const char *password) {
    if (-1 == sodium_init()) {
        return NULL;
    }

    char *hashed;
    if (NULL == (hashed = calloc(crypto_pwhash_STRBYTES, sizeof(char)))) {
        return NULL;
    }

    if (0 != crypto_pwhash_str(hashed, password, strlen(password),
                               crypto_pwhash_OPSLIMIT_SENSITIVE,
                               crypto_pwhash_MEMLIMIT_SENSITIVE)) {
        free(hashed);
        return NULL;
    }

    return hashed;
}
