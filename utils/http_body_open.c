//
// Created by 孙庆耀 on 2018/7/16.
//

#include "http_body_open.h"
#include "flash.h"


extern void http_body_open(struct kreq *req, enum khttp status) {
    khttp_head(req, kresps[KRESP_STATUS],
               "%s", khttps[status]);
    khttp_head(req, kresps[KRESP_CONTENT_TYPE],
               "%s", kmimetypes[req->mime]);
    khttp_head(req, "X-Content-Type-Options", "nosniff");
    khttp_head(req, "X-Frame-Options", "DENY");
    khttp_head(req, "X-XSS-Protection", "1; mode=block");
    khttp_body(req);
    khttp_puts(req, "<!DOCTYPE html>");
}
