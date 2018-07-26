//
// Created by 孙庆耀 on 2018/7/21.
//

#include "storage.h"
#include "../config.h"

#include <stdio.h>
#include <stdlib.h>


extern void set_value_integer(const char *key_prefix, const char *key,
                              long long value, unsigned long long expire) {

    redisContext *cxt = redisConnect(KERNOD_REDIS_HOSTNAME, KERNOD_REDIS_PORT);
    if (cxt == NULL || cxt->err) {
        if (cxt) {
            fprintf(stderr, "Redis error: %s\n", cxt->errstr);
        } else {
            fprintf(stderr, "Can't allocate redis context\n");
        }
        exit(EXIT_FAILURE);
    }

    redisReply *reply;
    reply = redisCommand(cxt, "SET %s:%s %lld EX %llu", key_prefix, key, value, expire);
    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis error: %s\n", reply->str);
        freeReplyObject(reply);
        exit(EXIT_FAILURE);
    }
    freeReplyObject(reply);

    redisFree(cxt);
}

extern struct redisReply *get_value(const char *key_prefix, const char *key) {

    redisContext *cxt = redisConnect(KERNOD_REDIS_HOSTNAME, KERNOD_REDIS_PORT);
    if (cxt == NULL || cxt->err) {
        if (cxt) {
            fprintf(stderr, "Redis error: %s\n", cxt->errstr);
        } else {
            fprintf(stderr, "Can't allocate redis context\n");
        }
        exit(EXIT_FAILURE);
    }

    redisReply *reply;
    reply = redisCommand(cxt, "GET %s:%s", key_prefix, key);
    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Redis error: %s\n", reply->str);
        freeReplyObject(reply);
        exit(EXIT_FAILURE);
    }

    redisFree(cxt);

    /* Remember to call freeReplyObject later from the caller */
    return reply;
}
