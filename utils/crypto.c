//
// Created by 孙庆耀 on 2018/7/19.
//

#include "../config.h"
#include <string.h>


extern char *hash_password_alloc(const char *password) {

    /* It is safe to call sodium_init() multiple times. */
    if (sodium_init() < 0) {
        return NULL;
    }

    char *hashed;
    if ((hashed = calloc(crypto_pwhash_STRBYTES, sizeof(char))) == NULL) {
        perror("calloc");
        return NULL;
    }

    if (crypto_pwhash_str(hashed, password, strlen(password),
                          KERNOD_PWHASH_OPSLIMIT, KERNOD_PWHASH_MEMLIMIT) != 0) {
        fprintf(stderr, "crypto_pwhash_str: out of memory\n");
        free(hashed);
        return NULL;
    }

    return hashed;
}
