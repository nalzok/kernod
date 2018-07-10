//
// Created by 孙庆耀 on 2018/7/10.
//

#include "process_index.h"

#include <kcgihtml.h>

extern enum khttp process_index(struct kreq *req) {
    struct khtmlreq htmlreq;
    khtml_open(&htmlreq, req, KHTML_PRETTY);
    khtml_elem(&htmlreq, KELEM_P);
    khtml_puts(&htmlreq, "hello, world");
    khtml_close(&htmlreq);
    return KHTTP_200;
}
