//
// Created by 孙庆耀 on 2018/7/19.
//

#include "../config.h"
#include <string.h>


extern char *hash_password_alloc(const char *password) {
    if (sodium_init() < 0) {
        return NULL;
    }

    char *hashed;
    if ((hashed = calloc(crypto_pwhash_STRBYTES, sizeof(char))) == NULL) {
        return NULL;
    }

    if (crypto_pwhash_str(hashed, password, strlen(password),
                          KERNOD_PWHASH_OPSLIMIT, KERNOD_PWHASH_MEMLIMIT) != 0) {
        free(hashed);
        return NULL;
    }

    return hashed;
}
