//
// Created by 孙庆耀 on 2018/7/14.
//

#include "flash.h"

#include <stdlib.h>
#include <string.h>


static const char *const msg_class[MSG_TYPE__MAX] = {
        "flash flash-success",
        "flash flash-danger",
        "flash flash-warning",
        "flash flash-info"
};

struct message {
    struct message *next;
    char *msg_data;
    enum msg_type type;
};

struct message *message_queue = NULL;

extern bool flash(const char *msg, const enum msg_type type) {
    struct message *message;
    if ((message = calloc(1, sizeof *message)) == NULL) {
        return false;
    }

    char *msg_data;
    if ((msg_data = strdup(msg)) == NULL) {
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
    while ((pmessage = message_queue) != NULL) {
        khtml_attr(htmlreq, KELEM_P,
                   KATTR_CLASS, msg_class[pmessage->type],
                   KATTR__MAX);
        khtml_puts(htmlreq, pmessage->msg_data);
        khtml_closeelem(htmlreq, 1);

        message_queue = pmessage->next;
        free(pmessage->msg_data);
        free(pmessage);
    }

    /* this is necessary due to a bug of kcgihtml */
    if (khtml_elemat(htmlreq) != pos) {
        khtml_closeto(htmlreq, pos);
    }
}
