//
// Created by 孙庆耀 on 2018/7/10.
//

#include "process_index.h"
#include "../../utils/flash.h"
#include "../../utils/http_body_open.h"
#include "../../utils/html_init.h"


extern enum khttp process_index(struct kreq *req) {
    http_body_open(req, KHTTP_200);

    struct khtmlreq htmlreq;
    khtml_open(&htmlreq, req, KHTML_PRETTY);
    html_init(&htmlreq);

    khtml_elem(&htmlreq, KELEM_P);
    khtml_puts(&htmlreq, "Welcome to Kernod");

    khtml_closeelem(&htmlreq, 0);
    khtml_close(&htmlreq);

    khttp_free(req);

    return KHTTP_200;
}
