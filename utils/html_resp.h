//
// Created by 孙庆耀 on 2018/7/18.
//

#ifndef KERNOD_HTML_RESP_H
#define KERNOD_HTML_RESP_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>
#include <kcgihtml.h>

extern struct khtmlreq *open_html_resp(struct kreq *req, enum khttp status, const char *title);

extern void free_html_resp(struct khtmlreq *htmlreq);

#endif //KERNOD_OPEN_HTML_RESP_H
