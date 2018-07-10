#include "main.h"
#include "index/process_index.h"
#include "auth/process_login.h"

#include <stdlib.h>


int main(void) {
    struct kreq req;
    struct kfcgi *fcgi;

    if (KCGI_OK != khttp_fcgi_init(&fcgi, keys, KEY__MAX,
                                   pages, PAGE__MAX, PAGE_INDEX)) {
        return EXIT_FAILURE;
    }

    while (KCGI_OK == khttp_fcgi_parse(fcgi, &req)) {
        enum khttp er;
        if (KHTTP_200 != (er = sanitise(&req))) {
            khttp_head(&req, kresps[KRESP_STATUS],
                       "%s", khttps[er]);
            khttp_head(&req, kresps[KRESP_CONTENT_TYPE],
                       "%s", kmimetypes[KMIME_TEXT_HTML]);
            khttp_body(&req);
            if (KMIME_TEXT_HTML == req.mime) {
                khttp_puts(&req, "Could not service content.");
            }
            khttp_free(&req);
            continue;
        }

        khttp_head(&req, kresps[KRESP_STATUS],
                   "%s", khttps[KHTTP_200]);
        khttp_head(&req, kresps[KRESP_CONTENT_TYPE],
                   "%s", kmimetypes[req.mime]);
        khttp_body(&req);
        dispatch(&req);
        khttp_free(&req);
    }

    khttp_fcgi_free(fcgi);
    return EXIT_SUCCESS;
}

static enum khttp sanitise(struct kreq *req) {
    if (PAGE__MAX == req->page) {
        return KHTTP_404;
    } else if (KMIME_TEXT_HTML != req->mime) {
        return KHTTP_404;
    } else if (KMETHOD_GET != req->method
               && !(KMETHOD_POST == req->method && PAGE_LOGIN == req->page)) {
        return KHTTP_405;
    }
    return KHTTP_200;
}

static enum khttp dispatch(struct kreq *req) {
    switch (req->page) {
        case PAGE_INDEX:
            return process_index(req);
        case PAGE_LOGIN:
            return process_login(req);
        default:
            return KHTTP_404;
    }
}
