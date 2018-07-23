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


struct session_t {
    int64_t user_id;
    char *username;
    char *email;
    char *join_ts;
};

extern struct session_t session;

extern struct session_t *init_session(struct kreq *req);

extern void free_session(void);

#endif //KERNOD_SESSION_H
