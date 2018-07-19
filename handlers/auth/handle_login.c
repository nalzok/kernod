//
// Created by 孙庆耀 on 2018/7/10.
//

#include "handle_login.h"
#include "../../utils/utils.h"
#include "../../config.h"

#include <ksql.h>
#include <string.h>


enum login_state {
    LOGIN_SUCCESS,
    LOGIN_SUCCESS_BUT_REHASH_FAILURE,
    LOGIN_FAILURE,
};

enum rehash_state {
    REHASH_SUCCESS,
    REHASH_FAILURE,
};

enum login_stmt {
    STMT_SELECT_PASSWORD,
    STMT_UPDATE_PASSWORD,
    STMT__MAX
};

static const char *const stmts[STMT__MAX] = {
        "SELECT password FROM users "
        "WHERE username = ?",
        "UPDATE users SET password = ? "
        "WHERE username = ?",
};

static void render_login_form(struct khtmlreq *htmlreq);

static enum login_state process_login_form(struct kreq *req);

static enum rehash_state rehash_password(struct ksql *sql, const char *username, const char *password);


extern enum khttp handle_login(struct kreq *req) {
    struct khtmlreq *htmlreq;

    switch (req->method) {

        case KMETHOD_GET:
            htmlreq = open_html_resp(req, KHTTP_200, "log in");

            khtml_elem(htmlreq, KELEM_H1);
            khtml_puts(htmlreq, "Login to continue");
            khtml_closeelem(htmlreq, 1);
            render_login_form(htmlreq);
            khtml_attr(htmlreq, KELEM_A,
                       KATTR_HREF, pages[PAGE_REGISTER],
                       KATTR__MAX);
            khtml_puts(htmlreq, "Don't have an account? Register now!");
            khtml_closeelem(htmlreq, 1);

            free_html_resp(htmlreq);

            khttp_free(req);
            return KHTTP_200;

        case KMETHOD_POST:
            if (process_login_form(req) == LOGIN_SUCCESS) {
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
            htmlreq = open_html_resp(req, KHTTP_405, khttps[KHTTP_405]);

            khtml_elem(htmlreq, KELEM_H1);
            khtml_puts(htmlreq, khttps[KHTTP_405]);
            khtml_closeelem(htmlreq, 1);

            free_html_resp(htmlreq);

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
               KATTR_FOR, "password",
               KATTR__MAX);
    khtml_puts(htmlreq, "Password");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_INPUT,
               KATTR_TYPE, "password",
               KATTR_ID, "password",
               KATTR_NAME, keys[KEY_PASSWORD].name,
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


static enum login_state process_login_form(struct kreq *req) {

    /* Check if all fields are present */

    struct kpair *pusername;
    if ((pusername = req->fieldmap[KEY_USERNAME]) == NULL) {
        flash("Invalid username", MSG_TYPE_DANGER);
        return LOGIN_FAILURE;
    }

    struct kpair *ppassword;
    if ((ppassword = req->fieldmap[KEY_PASSWORD]) == NULL) {
        flash("Invalid password", MSG_TYPE_DANGER);
        return LOGIN_FAILURE;
    }

    /* Initialize libsodium */

    if (sodium_init() < 0) {
        flash("libsodium error", MSG_TYPE_DANGER);
        return LOGIN_FAILURE;
    }

    /* Initialize database connection */

    struct ksqlcfg cfg;
    struct ksql *sql;

    ksql_cfg_defaults(&cfg);
    cfg.flags |= KSQL_FOREIGN_KEYS;
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    /* Get hashed password from database */

    struct ksqlstmt *select_password_stmt;
    enum ksqlc select_password_rv;

    ksql_stmt_alloc(sql, &select_password_stmt, NULL, STMT_SELECT_PASSWORD);
    ksql_bind_str(select_password_stmt, 0, pusername->parsed.s);
    select_password_rv = ksql_stmt_step(select_password_stmt);

    if (select_password_rv != KSQL_ROW) {
        flash("User doesn't exist", MSG_TYPE_DANGER);
        ksql_stmt_free(select_password_stmt);
        ksql_free(sql);
        return LOGIN_FAILURE;
    }

    const char *hashed_volatile;
    char *hashed;
    ksql_result_str(select_password_stmt, &hashed_volatile, 0);
    if ((hashed = strdup(hashed_volatile)) == NULL) {
        ksql_stmt_free(select_password_stmt);
        ksql_free(sql);
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    ksql_stmt_free(select_password_stmt);

    /* Validate password */

    if (crypto_pwhash_str_verify(hashed, ppassword->parsed.s,
                                 strlen(ppassword->parsed.s)) != 0) {
        flash("Wrong password", MSG_TYPE_DANGER);
        ksql_free(sql);
        return LOGIN_FAILURE;
    }

    flash("Welcome!", MSG_TYPE_SUCCESS);

    /* Rehash password if necessary */

    int needs_rehash = crypto_pwhash_str_needs_rehash(hashed,
                                                      KERNOD_PWHASH_OPSLIMIT,
                                                      KERNOD_PWHASH_MEMLIMIT);

    if (needs_rehash == 0) {
        /* No need to rehash */
        ksql_free(sql);
        return LOGIN_SUCCESS;
    }

    enum rehash_state rehash_password_rv;
    rehash_password_rv = rehash_password(sql, pusername->parsed.s, ppassword->parsed.s);

    ksql_free(sql);

    if (rehash_password_rv == REHASH_SUCCESS) {
        return LOGIN_SUCCESS;
    } else {
        /* failed to rehash, but we can do it the next time */
        return LOGIN_SUCCESS_BUT_REHASH_FAILURE;
    }
}

static enum rehash_state rehash_password(struct ksql *sql, const char *username, const char *password) {

    /* Compute a new hash */

    char *new_hashed = hash_password_alloc(password);
    if (new_hashed == NULL) {
        flash("Error computing password hash", MSG_TYPE_WARNING);
        return REHASH_FAILURE;
    }

    /* Update password hash */

    struct ksqlstmt *update_password_stmt;
    enum ksqlc update_password_rv;

    ksql_stmt_alloc(sql, &update_password_stmt, NULL, STMT_UPDATE_PASSWORD);
    ksql_bind_str(update_password_stmt, 0, new_hashed);
    ksql_bind_str(update_password_stmt, 1, username);
    update_password_rv = ksql_stmt_step(update_password_stmt);
    ksql_stmt_free(update_password_stmt);

    if (update_password_rv != KSQL_DONE) {
        flash("Cannot update password hash", MSG_TYPE_WARNING);
        return REHASH_FAILURE;
    }

    flash("Password rehashed", MSG_TYPE_SUCCESS);
    return REHASH_SUCCESS;
}
