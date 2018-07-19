//
// Created by 孙庆耀 on 2018/7/14.
//

#include "handle_register.h"
#include "../../utils/utils.h"
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

static enum register_state process_register_form(struct kreq *req);

static char *hash_password_alloc(const char *password);


extern enum khttp handle_register(struct kreq *req) {
    struct khtmlreq *htmlreq;

    switch (req->method) {

        case KMETHOD_GET:
            htmlreq = open_html_resp(req, KHTTP_200, "register");

            khtml_elem(htmlreq, KELEM_H1);
            khtml_puts(htmlreq, "Join now!");
            khtml_closeelem(htmlreq, 1);
            render_register_form(htmlreq);
            khtml_attr(htmlreq, KELEM_A,
                       KATTR_HREF, pages[PAGE_LOGIN],
                       KATTR__MAX);
            khtml_puts(htmlreq, "Already have an account? Log in!");
            khtml_closeelem(htmlreq, 1);

            free_html_resp(htmlreq);

            khttp_free(req);
            return KHTTP_200;

        case KMETHOD_POST:
            if (process_register_form(req) == REG_SUCCESS) {
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
            htmlreq = open_html_resp(req, KHTTP_405, khttps[KHTTP_405]);

            khtml_elem(htmlreq, KELEM_H1);
            khtml_puts(htmlreq, khttps[KHTTP_405]);
            khtml_closeelem(htmlreq, 1);

            free_html_resp(htmlreq);

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
               KATTR_FOR, "username",
               KATTR__MAX);
    khtml_puts(htmlreq, "Username");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "text",
               KATTR_ID, "username",
               KATTR_NAME, keys[KEY_USERNAME].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "email",
               KATTR__MAX);
    khtml_puts(htmlreq, "Email");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "text",
               KATTR_ID, "email",
               KATTR_NAME, keys[KEY_EMAIL].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "password",
               KATTR__MAX);
    khtml_puts(htmlreq, "Password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "password",
               KATTR_NAME, keys[KEY_PASSWORD].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "password2",
               KATTR__MAX);
    khtml_puts(htmlreq, "Retype password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "password2",
               KATTR_NAME, keys[KEY_PASSWORD2].name,
               KATTR__MAX);

    khtml_elem(htmlreq, KELEM_P);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "submit",
               KATTR_VALUE, "Submit",
               KATTR__MAX);
    khtml_closeelem(htmlreq, 1);

    /* this is necessary due to a bug of kcgihtml */
    if (khtml_elemat(htmlreq) != pos) {
        khtml_closeto(htmlreq, pos);
    }
}

static enum register_state process_register_form(struct kreq *req) {

    /* Check if all fields are present */

    struct kpair *pusername;
    if ((pusername = req->fieldmap[KEY_USERNAME]) == NULL) {
        flash("Invalid username", MSG_TYPE_DANGER);
        return REG_FAILURE;
    }

    struct kpair *pemail;
    if ((pemail = req->fieldmap[KEY_EMAIL]) == NULL) {
        flash("Invalid email", MSG_TYPE_DANGER);
        return REG_FAILURE;
    }

    struct kpair *ppassword, *ppassword2;
    if ((ppassword = req->fieldmap[KEY_PASSWORD]) == NULL
        || (ppassword2 = req->fieldmap[KEY_PASSWORD2]) == NULL) {
        flash("Invalid password", MSG_TYPE_DANGER);
        return REG_FAILURE;
    }

    /* Check if password matches password2 */

    if (strcmp(ppassword->parsed.s,
               ppassword2->parsed.s) != 0) {
        flash("Passwords mismatch", MSG_TYPE_DANGER);
        return REG_FAILURE;
    }

    /* initialize database connection */

    struct ksqlcfg cfg;
    struct ksql *sql;

    ksql_cfg_defaults(&cfg);
    cfg.flags |= KSQL_FOREIGN_KEYS;
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    /* Check if username already taken */

    struct ksqlstmt *select_username_stmt;
    enum ksqlc select_username_rv;

    ksql_stmt_alloc(sql, &select_username_stmt, NULL, STMT_SELECT_USER);
    ksql_bind_str(select_username_stmt, 0, pusername->parsed.s);
    select_username_rv = ksql_stmt_step(select_username_stmt);
    ksql_stmt_free(select_username_stmt);

    if (select_username_rv == KSQL_ROW) {
        flash("Username already taken", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    } else if (select_username_rv != KSQL_DONE) {
        flash("Database error", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    }

    /* Check if email already taken */

    struct ksqlstmt *select_email_stmt;
    enum ksqlc select_email_rv;

    ksql_stmt_alloc(sql, &select_email_stmt, NULL, STMT_SELECT_EMAIL);
    ksql_bind_str(select_email_stmt, 0, pemail->parsed.s);
    select_email_rv = ksql_stmt_step(select_email_stmt);
    ksql_stmt_free(select_email_stmt);

    if (select_email_rv == KSQL_ROW) {
        flash("Email already taken", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    } else if (select_email_rv != KSQL_DONE) {
        flash("Database error", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    }

    /* All fields validated, now create a new user */

    char *hashed = hash_password_alloc(ppassword->parsed.s);
    if (hashed == NULL) {
        flash("Error hashing password", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    }

    struct ksqlstmt *insert_user_stmt;
    ksql_stmt_alloc(sql, &insert_user_stmt, NULL, STMT_INSERT_USER);
    ksql_bind_str(insert_user_stmt, 0, pusername->parsed.s);
    ksql_bind_str(insert_user_stmt, 1, pemail->parsed.s);
    ksql_bind_str(insert_user_stmt, 2, hashed);
    free(hashed);

    if (ksql_stmt_step(insert_user_stmt) == KSQL_DONE) {
        flash("User created", MSG_TYPE_SUCCESS);
        ksql_stmt_free(insert_user_stmt);
        ksql_free(sql);
        return REG_SUCCESS;
    } else {
        flash("Database error", MSG_TYPE_DANGER);
        ksql_stmt_free(insert_user_stmt);
        ksql_free(sql);
        return REG_FAILURE;
    }
}

static char *hash_password_alloc(const char *password) {
    if (sodium_init() < 0) {
        return NULL;
    }

    char *hashed;
    if ((hashed = calloc(crypto_pwhash_STRBYTES, sizeof(char))) == NULL) {
        return NULL;
    }

    if (crypto_pwhash_str(hashed, password, strlen(password),
                          crypto_pwhash_OPSLIMIT_SENSITIVE,
                          crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
        free(hashed);
        return NULL;
    }

    return hashed;
}
