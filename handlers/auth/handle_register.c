//
// Created by 孙庆耀 on 2018/7/14.
//

#include "handle_register.h"
#include "../../utils/utils.h"
#include "../../config.h"

#include <ksql.h>
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

static void insert_register_form(struct khtmlreq *htmlreq);

static enum register_state process_register_form(struct kreq *req);


extern enum khttp handle_register(struct kreq *req) {
    struct khtmlreq *htmlreq = NULL;

    switch (req->method) {

        case KMETHOD_GET:
            htmlreq = html_resp_alloc(req, KHTTP_200, "register");

            khtml_elem(htmlreq, KELEM_H1);
            khtml_puts(htmlreq, "Join now!");
            khtml_closeelem(htmlreq, 1);
            insert_register_form(htmlreq);
            khtml_attr(htmlreq, KELEM_A,
                       KATTR_HREF, pages[PAGE_LOGIN],
                       KATTR__MAX);
            khtml_puts(htmlreq, "Already have an account? Log in!");
            khtml_closeelem(htmlreq, 1);

            free_html_resp(htmlreq);

            khttp_free(req);
            return KHTTP_200;

        case KMETHOD_POST:
            /* Post/Redirect/Get pattern */
            if (process_register_form(req) == REG_SUCCESS) {
                redirect_resp(req, pages[PAGE_LOGIN]);
            } else {
                redirect_resp(req, pages[PAGE_REGISTER]);
            }
            return KHTTP_303;

        default:
            status_only_resp(req, KHTTP_405);
            return KHTTP_405;
    }
}

static void insert_register_form(struct khtmlreq *htmlreq) {
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
               KATTR_NAME, key_cookies[KEY_USERNAME].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "email",
               KATTR__MAX);
    khtml_puts(htmlreq, "Email");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "text",
               KATTR_ID, "email",
               KATTR_NAME, key_cookies[KEY_EMAIL].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "password",
               KATTR__MAX);
    khtml_puts(htmlreq, "Password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "password",
               KATTR_NAME, key_cookies[KEY_PASSWORD].name,
               KATTR__MAX);

    khtml_attr(htmlreq, KELEM_LABEL,
               KATTR_FOR, "password2",
               KATTR__MAX);
    khtml_puts(htmlreq, "Retype password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "password2",
               KATTR_NAME, key_cookies[KEY_PASSWORD2].name,
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
    ksql_cfg_defaults(&cfg);
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    struct ksql *sql;
    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    /* Check if username already taken */

    struct ksqlstmt *select_username_stmt;
    ksql_stmt_alloc(sql, &select_username_stmt, NULL, STMT_SELECT_USER);
    ksql_bind_str(select_username_stmt, 0, pusername->parsed.s);

    enum ksqlc select_username_rv;
    select_username_rv = ksql_stmt_step(select_username_stmt);
    ksql_stmt_free(select_username_stmt);

    if (select_username_rv == KSQL_ROW) {
        flash("Username already taken", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    } else if (select_username_rv != KSQL_DONE) {
        flash("Unknown database error", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    }

    /* Check if email already taken */

    struct ksqlstmt *select_email_stmt;
    ksql_stmt_alloc(sql, &select_email_stmt, NULL, STMT_SELECT_EMAIL);
    ksql_bind_str(select_email_stmt, 0, pemail->parsed.s);

    enum ksqlc select_email_rv;
    select_email_rv = ksql_stmt_step(select_email_stmt);
    ksql_stmt_free(select_email_stmt);

    if (select_email_rv == KSQL_ROW) {
        flash("Email already taken", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    } else if (select_email_rv != KSQL_DONE) {
        flash("Unknown database error", MSG_TYPE_DANGER);
        ksql_free(sql);
        return REG_FAILURE;
    }

    /* All fields validated, now create a new user */

    char *hashed = hash_password_alloc(ppassword->parsed.s);
    if (hashed == NULL) {
        fprintf(stderr, "Error hashing password\n");
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
