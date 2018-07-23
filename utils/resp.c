//
// Created by 孙庆耀 on 2018/7/18.
//

#include "resp.h"
#include "flash.h"
#include <stdlib.h>


static void open_http_body(struct kreq *req, enum khttp status);

static void init_html(struct khtmlreq *htmlreq, const char *title);


extern struct khtmlreq *html_resp_alloc(struct kreq *req, enum khttp status, const char *title) {
    open_http_body(req, status);
    khttp_puts(req, "<!DOCTYPE html>");

    struct khtmlreq *htmlreq;
    if ((htmlreq = calloc(1, sizeof *htmlreq)) == NULL) {
        exit(EXIT_FAILURE);
    }

    khtml_open(htmlreq, req, KHTML_PRETTY);
    init_html(htmlreq, title);

    return htmlreq;
}

extern void free_html_resp(struct khtmlreq *htmlreq) {
    khtml_closeelem(htmlreq, 0);
    khtml_close(htmlreq);
    free(htmlreq);
}

static void open_http_body(struct kreq *req, enum khttp status) {
    khttp_head(req, kresps[KRESP_STATUS],
               "%s", khttps[status]);
    khttp_head(req, kresps[KRESP_CONTENT_TYPE],
               "%s", kmimetypes[req->mime]);
    khttp_head(req, "X-Content-Type-Options", "nosniff");
    khttp_head(req, "X-Frame-Options", "DENY");
    khttp_head(req, "X-XSS-Protection", "1; mode=block");
    khttp_body(req);
}

static void init_html(struct khtmlreq *htmlreq, const char *title) {
    khtml_elem(htmlreq, KELEM_HTML);

    khtml_elem(htmlreq, KELEM_HEAD);
    khtml_attr(htmlreq, KELEM_META,
               KATTR_CHARSET, "utf-8",
               KATTR__MAX);
    khtml_attr(htmlreq, KELEM_META,
               KATTR_NAME, "viewport",
               KATTR_CONTENT, "width=device-width, initial-scale=1, shrink-to-fit=no",
               KATTR__MAX);
    khtml_elem(htmlreq, KELEM_TITLE);
    khtml_puts(htmlreq, "Kernod - ");
    khtml_puts(htmlreq, title);
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_LINK,
               KATTR_REL, "stylesheet",
               KATTR_HREF, "/static/css/bootstrap-reboot.css",
               KATTR__MAX);
    khtml_closeelem(htmlreq, 1);

    khtml_elem(htmlreq, KELEM_BODY);
    get_flashed_messages(htmlreq);
}

extern void redirect_resp(struct kreq *req, const char *loc) {
    khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[KHTTP_303]);
    khttp_head(req, kresps[KRESP_LOCATION], "%s", loc);
    khttp_body(req);
    khttp_free(req);
}

extern void status_only_resp(struct kreq *req, enum khttp status) {
    khttp_head(req, kresps[KRESP_STATUS], "%s", khttps[status]);
    khttp_body(req);
    khttp_puts(req, khttps[status]);
    khttp_free(req);
}
