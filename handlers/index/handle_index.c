//
// Created by 孙庆耀 on 2018/7/10.
//

#include "handle_index.h"
#include "../../utils/utils.h"
#include "../../config.h"


extern enum khttp handle_index(struct kreq *req) {
    struct khtmlreq *htmlreq;
    htmlreq = html_resp_alloc(req, KHTTP_200, "home");

    khtml_elem(htmlreq, KELEM_P);
    if (session.user_id != 0) {
        khtml_puts(htmlreq, "Welcome back to Kernod, ");
        khtml_puts(htmlreq, session.username);
    } else {
        khtml_puts(htmlreq, "Welcome to Kernod!");
    }
    khtml_closeelem(htmlreq, 1);

    khtml_elem(htmlreq, KELEM_P);
    khtml_attr(htmlreq, KELEM_A,
               KATTR_HREF, pages[PAGE_LOGIN],
               KATTR__MAX);
    khtml_puts(htmlreq, "Log In");
    khtml_closeelem(htmlreq, 1);
    khtml_attr(htmlreq, KELEM_A,
               KATTR_HREF, pages[PAGE_REGISTER],
               KATTR__MAX);
    khtml_puts(htmlreq, "Register");
    khtml_closeelem(htmlreq, 1);
    khtml_closeelem(htmlreq, 1);

    free_html_resp(htmlreq);

    khttp_free(req);

    return KHTTP_200;
}
