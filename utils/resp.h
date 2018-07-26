//
// Created by 孙庆耀 on 2018/7/18.
//

#ifndef KERNOD_RESP_H
#define KERNOD_RESP_H

#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <kcgi.h>
#include <kcgihtml.h>


extern struct khtmlreq *html_resp_alloc(struct kreq *req, enum khttp status, const char *title);

extern void free_html_resp(struct khtmlreq *htmlreq);

extern void redirect_resp(struct kreq *req, const char *loc);

extern void status_only_resp(struct kreq *req, enum khttp status);

#endif //KERNOD_OPEN_RESP_H
