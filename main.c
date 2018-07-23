#include "handlers/handlers.h"
#include "utils/utils.h"
#include "config.h"


static enum khttp sanitise(struct kreq *req);

static enum khttp dispatch(struct kreq *req);

int main(void) {
    struct kreq req;
    struct kfcgi *fcgi;

    if (khttp_fcgi_init(&fcgi, key_cookies, KEY_COOKIE__MAX,
                        pages, PAGE__MAX, PAGE_INDEX) != KCGI_OK) {
        return EXIT_FAILURE;
    }

    while (khttp_fcgi_parse(fcgi, &req) == KCGI_OK) {
        init_session(&req);

        if (sanitise(&req) != KHTTP_200) {
            continue;
        }

        dispatch(&req);
    }

    khttp_fcgi_free(fcgi);
    return EXIT_SUCCESS;
}

static enum khttp sanitise(struct kreq *req) {
    if (req->page == PAGE__MAX) {
        status_only_resp(req, KHTTP_404);
        return KHTTP_404;
    } else if (req->mime != KMIME_TEXT_HTML) {
        status_only_resp(req, KHTTP_404);
        return KHTTP_404;
    } else if (require_login[req->page] && session.user_id == 0) {
        redirect_resp(req, pages[PAGE_LOGIN]);
        return KHTTP_303;
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
            return KHTTP_404;
    }
}
