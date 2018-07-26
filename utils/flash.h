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


enum msg_type {
    MSG_TYPE_SUCCESS,
    MSG_TYPE_DANGER,
    MSG_TYPE_WARNING,
    MSG_TYPE_INFO,
    MSG_TYPE__MAX
};

extern void flash(const char *msg, enum msg_type type);

extern void get_flashed_messages(struct khtmlreq *htmlreq);

#endif //KERNOD_FLASH_H
