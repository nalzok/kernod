//
// Created by 孙庆耀 on 2018/7/14.
//

#ifndef KERNOD_FLASH_H
#define KERNOD_FLASH_H

struct messages {
    struct messages *next;
    const char *message;
};

extern struct messages *message_queue;

extern void flash(const char *message);

extern void get_flashed_messages(void);

#endif //KERNOD_FLASH_H
