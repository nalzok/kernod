//
// Created by 孙庆耀 on 2018/7/14.
//

#ifndef KERNOD_PROCESS_REGISTER_H
#define KERNOD_PROCESS_REGISTER_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>


extern enum khttp process_register(struct kreq *req);

#endif //KERNOD_PROCESS_REGISTER_H
