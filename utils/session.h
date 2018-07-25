//
// Created by 孙庆耀 on 2018/7/21.
//

#ifndef KERNOD_SESSION_H
#define KERNOD_SESSION_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>


enum session_init_state {
    SESSION_INIT_SUCCESS,
    SESSION_INIT_FAILURE
};

struct session_t {
    struct user_t *user;
};

struct user_t {
    int64_t user_id;
    char *username;
    char *email;
    char *join_ts;
};

extern enum session_init_state populate_session(struct kreq *req);

extern const struct user_t *current_user(void);

extern void free_session(void);

#endif //KERNOD_SESSION_H
