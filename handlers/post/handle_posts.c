//
// Created by 孙庆耀 on 2018/7/25.
//

#include "handle_posts.h"
#include "../../utils/utils.h"
#include "../../config.h"

#include <ksql.h>


struct post {
    struct post *next;
    char *title;
    char *excerpt;
    char *author_name;
};

enum posts_stmt {
    STMT_SELECT_POST,
    STMT__MAX
};

static const char *const stmts[STMT__MAX] = {
        "SELECT title, body, username FROM questions JOIN users "
        "WHERE questions.author_id = users.user_id "
        "ORDER BY DATETIME(post_ts) DESC "
        "LIMIT ? OFFSET ?",
};

static struct post *get_posts(int64_t per_page, int64_t page);

static void free_posts(struct post *post_list);


extern enum khttp handle_posts(struct kreq *req) {
    struct khtmlreq *htmlreq;
    htmlreq = html_resp_alloc(req, KHTTP_200, "Posts");

    khtml_elem(htmlreq, KELEM_H1);
    khtml_puts(htmlreq, "All Posts");
    khtml_closeelem(htmlreq, 1);

    /* Get pagination info from request, use default on failure */

    int64_t per_page = KERNOD_PAGINATION_PER_PAGE;
    struct kpair *pper_page;
    if ((pper_page = req->cookiemap[COOKIE_PER_PAGE]) != NULL) {
        per_page = pper_page->parsed.i;
    }

    int64_t page = 1;
    struct kpair *ppage;
    if ((ppage = req->fieldmap[KEY_PAGE]) != NULL) {
        page = ppage->parsed.i;
    }

    khtml_elem(htmlreq, KELEM_UL);
    struct post *post_list = get_posts(per_page, page);
    struct post *post_iter;
    for (post_iter = post_list; post_iter != NULL; post_iter = post_iter->next) {
        khtml_elem(htmlreq, KELEM_LI);

        khtml_elem(htmlreq, KELEM_H3);
        khtml_puts(htmlreq, post_iter->title);
        khtml_closeelem(htmlreq, 1);

        khtml_elem(htmlreq, KELEM_P);
        khtml_puts(htmlreq, post_iter->excerpt);
        khtml_closeelem(htmlreq, 1);

        khtml_elem(htmlreq, KELEM_SMALL);
        khtml_puts(htmlreq, post_iter->author_name);
        khtml_closeelem(htmlreq, 1);

        khtml_closeelem(htmlreq, 1);
    }
    free_posts(post_list);
    khtml_closeelem(htmlreq, 1);

    free_html_resp(htmlreq);

    khttp_free(req);

    return KHTTP_200;
}

static struct post *get_posts(int64_t per_page, int64_t page) {

    /* Initialize database connection */

    struct ksqlcfg cfg;
    ksql_cfg_defaults(&cfg);
    cfg.flags |= KSQL_FOREIGN_KEYS;
    cfg.stmts.stmts = stmts;
    cfg.stmts.stmtsz = STMT__MAX;

    struct ksql *sql;
    sql = ksql_alloc_child(&cfg, NULL, NULL);
    ksql_open(sql, "kernod.sqlite");

    /* Get post info from database */

    struct ksqlstmt *select_post_stmt;
    ksql_stmt_alloc(sql, &select_post_stmt, NULL, STMT_SELECT_POST);
    ksql_bind_int(select_post_stmt, 0, per_page);
    ksql_bind_int(select_post_stmt, 1, (page - 1) * per_page);

    enum ksqlc select_post_rv = ksql_stmt_step(select_post_stmt);

    struct post *post_list, *last_post;
    for (last_post = post_list = NULL; select_post_rv == KSQL_ROW; select_post_rv = ksql_stmt_step(select_post_stmt)) {
        struct post *new_post = calloc(1, sizeof *new_post);
        if (new_post == NULL) {
            perror("calloc");
            ksql_stmt_free(select_post_stmt);
            ksql_free(sql);
            exit(EXIT_FAILURE);
        }

        new_post->next = NULL;
        ksql_result_str_alloc(select_post_stmt, &(new_post->title), 0);
        ksql_result_str_alloc(select_post_stmt, &(new_post->excerpt), 1);
        ksql_result_str_alloc(select_post_stmt, &(new_post->author_name), 2);

        if (last_post == NULL) {
            /* First iteration */
            post_list = new_post;
        } else {
            /* Subsequent iterations */
            last_post->next = new_post;
        }
        last_post = new_post;
    }

    ksql_stmt_free(select_post_stmt);
    ksql_free(sql);

    return post_list;
}

static void free_posts(struct post *post_list) {
    while (post_list != NULL) {
        free(post_list->title);
        free(post_list->excerpt);
        free(post_list->author_name);
        struct post *next_post = post_list->next;
        free(post_list);
        post_list = next_post;
    }
}