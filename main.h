//
// Created by 孙庆耀 on 2018/7/10.
//

#ifndef KERNOD_MAIN_H
#define KERNOD_MAIN_H

#include "config.h"

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>

static enum khttp sanitise(struct kreq *req);

static enum khttp dispatch(struct kreq *req);


#endif //KERNOD_MAIN_H
