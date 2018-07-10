//
// Created by 孙庆耀 on 2018/7/10.
//

#ifndef KERNOD_CONFIG_H
#define KERNOD_CONFIG_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>

enum page {
    PAGE_INDEX,
    PAGE_LOGIN,
    PAGE__MAX
};

static const char *const pages[PAGE__MAX] = {
        "index",
        "login",
};

enum key {
    KEY_USERNAME,
    KEY_PASSWORD,
    KEY__MAX
};

static const struct kvalid keys[KEY__MAX] = {
        {kvalid_stringne, "username"},
        {kvalid_stringne, "password"},
};

#endif //KERNOD_CONFIG_H
