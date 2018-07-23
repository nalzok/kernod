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
#include <sodium.h>
#include <stdbool.h>


/* Expiration time for sort-term session management cookies in seconds;
 * DO NOT use a large value */
#define KERNOD_SESSION_EXPIRE_SECONDS 3600LLU

#define KERNOD_REDIS_HOSTNAME "127.0.0.1"
#define KERNOD_REDIS_PORT 6379

#define KERNOD_PWHASH_OPSLIMIT crypto_pwhash_OPSLIMIT_MODERATE
#define KERNOD_PWHASH_MEMLIMIT crypto_pwhash_MEMLIMIT_MODERATE

enum page {
    PAGE_INDEX,
    PAGE_LOGIN,
    PAGE_REGISTER,
    PAGE__MAX
};

static const char *const pages[PAGE__MAX] = {
        "index",
        "login",
        "register",
};

static const bool require_login[PAGE__MAX] = {
        false,
        false,
        true,
};

enum key_cookie {
    KEY_USERNAME,
    KEY_EMAIL,
    KEY_PASSWORD,
    KEY_PASSWORD2,
    KEY_REMEMBER_ME,
    COOKIE_SESSION_ID,
    KEY_COOKIE__MAX
};

static const struct kvalid key_cookies[KEY_COOKIE__MAX] = {
        /* form inputs */
        {kvalid_stringne, "username"},
        {kvalid_email,    "email"},
        {kvalid_stringne, "password"},
        {kvalid_stringne, "password2"},
        {kvalid_stringne, "remember-me"},

        /* cookies */
        {kvalid_stringne, "session_id"},
};

#endif //KERNOD_CONFIG_H
