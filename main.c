#include "handlers/handlers.h"
#include "config.h"

#include <stdlib.h>
#include <assert.h>


static enum khttp sanitise(struct kreq *req);

static enum khttp dispatch(struct kreq *req);

int main(void) {
    struct kreq req;
    struct kfcgi *fcgi;

    if (khttp_fcgi_init(&fcgi, keys, KEY__MAX,
                        pages, PAGE__MAX, PAGE_INDEX) != KCGI_OK) {
        return EXIT_FAILURE;
    }

    while (khttp_fcgi_parse(fcgi, &req) == KCGI_OK) {
        enum khttp er;
        if ((er = sanitise(&req)) != KHTTP_200) {
            khttp_head(&req, kresps[KRESP_STATUS],
                       "%s", khttps[er]);
            khttp_head(&req, kresps[KRESP_CONTENT_TYPE],
                       "%s", kmimetypes[KMIME_TEXT_HTML]);
            khttp_body(&req);
            if (req.mime == KMIME_TEXT_HTML) {
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
    if (req->page == PAGE__MAX) {
        return KHTTP_404;
    } else if (req->mime != KMIME_TEXT_HTML) {
        return KHTTP_404;
    }
    return KHTTP_200;
}

static enum khttp dispatch(struct kreq *req) {
    switch (req->page) {
        case PAGE_INDEX:
            return handle_index(req);
        case PAGE_LOGIN:
            return handle_login(req);
        case PAGE_REGISTER:
            return handle_register(req);
        default:
            assert(0);
            return KHTTP_404;
    }
}
