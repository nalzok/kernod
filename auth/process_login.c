//
// Created by 孙庆耀 on 2018/7/10.
//

#include "process_login.h"
#include "../config.h"

#include <kcgihtml.h>

extern enum khttp process_login(struct kreq *req) {
    struct khtmlreq htmlreq;
    khtml_open(&htmlreq, req, KHTML_PRETTY);
    khtml_elem(&htmlreq, KELEM_P);
    khtml_attr(&htmlreq, KELEM_FORM,
               KATTR_METHOD, "post",
               KATTR_ENCTYPE, "multipart/form-data",
               KATTR_ACTION, "login",
               KATTR__MAX);
    khtml_elem(&htmlreq, KELEM_FIELDSET);
    khtml_elem(&htmlreq, KELEM_LEGEND);

    khtml_puts(&htmlreq, "Post (multipart)");
    khtml_closeelem(&htmlreq, 1);
    khtml_elem(&htmlreq, KELEM_P);
    khtml_attr(&htmlreq, KELEM_INPUT,
               KATTR_TYPE, "number",
               KATTR_NAME, keys[KEY_USERNAME].name,
               KATTR__MAX);
    khtml_closeelem(&htmlreq, 1);
    khtml_elem(&htmlreq, KELEM_P);
    khtml_attr(&htmlreq, KELEM_INPUT,
               KATTR_TYPE, "file",
               KATTR_MULTIPLE, "",
               KATTR_NAME, keys[KEY_PASSWORD].name,
               KATTR__MAX);
    khtml_close(&htmlreq);
    return KHTTP_200;
}
