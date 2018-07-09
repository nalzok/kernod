#include <sys/types.h> /* size_t, ssize_t */
#include <stdarg.h> /* va_list */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int64_t */
#include <stdlib.h>
#include <kcgi.h>

int main(void) {
    struct kreq r;
    struct kfcgi *fcgi;
    const char *page = "index";

    if (KCGI_OK != khttp_fcgi_init(&fcgi, NULL, 0, &page, 1, 0))
        return(EXIT_FAILURE);

    while (KCGI_OK == khttp_fcgi_parse(fcgi, &r)) {
        khttp_head(&r, kresps[KRESP_STATUS],
                   "%s", khttps[KHTTP_200]);
        khttp_head(&r, kresps[KRESP_CONTENT_TYPE],
                   "%s", kmimetypes[r.mime]);
        khttp_body(&r);
        khttp_puts(&r, "Hello, world!");
        khttp_free(&r);
    }

    khttp_fcgi_free(fcgi);
    return(EXIT_SUCCESS);
}
