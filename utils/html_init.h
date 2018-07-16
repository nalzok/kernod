//
// Created by 孙庆耀 on 2018/7/16.
//

#ifndef KERNOD_HTML_INIT_H
#define KERNOD_HTML_INIT_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>
#include <kcgihtml.h>

extern void html_init(struct khtmlreq *htmlreq);

#endif //KERNOD_HTML_INIT_H
