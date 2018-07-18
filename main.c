#include "handlers/index/process_index.h"
#include "handlers/auth/handle_login.h"
#include "handlers/auth/handle_register.h"
#include "config.h"

#include <stdlib.h>
#include <assert.h>


static enum khttp sanitise(struct kreq *req);

static enum khttp dispatch(struct kreq *req);

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

        dispatch(&req);
    }

    khttp_fcgi_free(fcgi);
    return EXIT_SUCCESS;
}

static enum khttp sanitise(struct kreq *req) {
    if (PAGE__MAX == req->page) {
        return KHTTP_404;
    } else if (KMIME_TEXT_HTML != req->mime) {
        return KHTTP_404;
    }
    return KHTTP_200;
}

static enum khttp dispatch(struct kreq *req) {
    switch (req->page) {
        case PAGE_INDEX:
            return process_index(req);
        case PAGE_LOGIN:
            return handle_login(req);
        case PAGE_REGISTER:
            return handle_register(req);
        default:
            assert(0);
            return KHTTP_404;
    }
}
