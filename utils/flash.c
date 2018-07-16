//
// Created by 孙庆耀 on 2018/7/14.
//

#include "flash.h"

#include <stdlib.h>
#include <string.h>


static const char *const msg_class[MSG_TYPE__MAX] = {
        "info",
        "warning",
        "error",
        "critical"
};

struct message {
    struct message *next;
    char *msg_data;
    enum msg_type type;
};

struct message *message_queue = NULL;

extern bool flash(const char *msg, const enum msg_type type) {
    struct message *message;
    if (NULL == (message = calloc(1, sizeof *message))) {
        return false;
    }

    char *msg_data;
    if (NULL == (msg_data = strdup(msg))) {
        free(message);
        return false;
    }

    message->next = message_queue;
    message->msg_data = msg_data;
    message->type = type;
    message_queue = message;
    return true;
}

extern void get_flashed_messages(struct khtmlreq *htmlreq) {
    size_t pos = khtml_elemat(htmlreq);

    struct message *pmessage;
    while (NULL != (pmessage = message_queue)) {
        khtml_attr(htmlreq, KELEM_P,
                   KATTR_CLASS, msg_class[pmessage->type],
                   KATTR__MAX);
        khtml_puts(htmlreq, pmessage->msg_data);
        khtml_closeelem(htmlreq, 1);

        message_queue = pmessage->next;
        free(pmessage->msg_data);
        free(pmessage);
    }

    khtml_closeto(htmlreq, pos);
}
