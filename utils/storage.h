//
// Created by 孙庆耀 on 2018/7/21.
//

#ifndef KERNOD_STORAGE_H
#define KERNOD_STORAGE_H

#include <hiredis/hiredis.h>


extern void set_value_integer(const char *key_prefix, const char *key, long long value, unsigned long long expire);

extern struct redisReply *get_value(const char *key_prefix, const char *key);

#endif //KERNOD_STORAGE_H
