//
// Created by 孙庆耀 on 2018/7/10.
//

#ifndef KERNOD_HANDLE_LOGIN_H
#define KERNOD_HANDLE_LOGIN_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>


extern enum khttp handle_login(struct kreq *req);

#endif //KERNOD_HANDLE_LOGIN_H
