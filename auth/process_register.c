//
// Created by 孙庆耀 on 2018/7/14.
//

#include "process_register.h"


extern enum khttp process_register(struct kreq *req) {
    (void) req;
    return KHTTP_200;
}
