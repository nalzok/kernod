//
// Created by 孙庆耀 on 2018/7/16.
//

#include "html_init.h"
#include "flash.h"


extern void html_init(struct khtmlreq *htmlreq) {
    khtml_elem(htmlreq, KELEM_HTML);

    khtml_elem(htmlreq, KELEM_HEAD);
    khtml_attr(htmlreq, KELEM_META,
               KATTR_CHARSET, "utf-8",
               KATTR__MAX);
    khtml_attr(htmlreq, KELEM_META,
               KATTR_NAME, "viewport",
               KATTR_CONTENT, "width=device-width, initial-scale=1, shrink-to-fit=no",
               KATTR__MAX);
    khtml_attr(htmlreq, KELEM_LINK,
               KATTR_REL, "stylesheet",
               KATTR_HREF, "/static/css/styles.css",
               KATTR__MAX);
    khtml_closeelem(htmlreq, 1);

    khtml_elem(htmlreq, KELEM_BODY);
    get_flashed_messages(htmlreq);
}
