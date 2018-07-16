//
// Created by 孙庆耀 on 2018/7/14.
//

#ifndef KERNOD_FLASH_H
#define KERNOD_FLASH_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>
#include <kcgihtml.h>
#include <stddef.h>
#include <stdbool.h>


enum msg_type {
    MSG_TYPE_INFO,
    MSG_TYPE_WARNING,
    MSG_TYPE_ERROR,
    MSG_TYPE_CRITICAL,
    MSG_TYPE__MAX
};

extern struct message *message_queue;

extern bool flash(const char *msg, enum msg_type type);

extern void get_flashed_messages(struct khtmlreq *htmlreq);

#endif //KERNOD_FLASH_H
