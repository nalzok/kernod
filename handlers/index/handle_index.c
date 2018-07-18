//
// Created by 孙庆耀 on 2018/7/10.
//

#include "handle_index.h"
#include "../../utils/utils.h"
#include "../../config.h"


extern enum khttp handle_index(struct kreq *req) {
    struct khtmlreq *htmlreq;
    htmlreq = open_html_resp(req, KHTTP_200, "Home");

    khtml_elem(htmlreq, KELEM_P);
    khtml_puts(htmlreq, "Welcome to Kernod!");
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
