#include <cstdio>
#include <cstdlib>
#include <new>

#include "io/base64.h"

char *io::b64_encode(const char *src, size_t length) {
    // (length * 4 / 3) rounded up to nearest multiple of 4
    size_t encoded_length = ((length<<2)/3+3)&-4;
    char *buffer = (char*)malloc((sizeof *buffer) * encoded_length + 1);

    if (buffer == NULL) {
        printf("Call to malloc() in io::b64_encode() failed. Exiting...");
        exit(EXIT_FAILURE);
    }

    char *p = buffer;
    size_t i;

    for(i = 0; i < length-2; i += 3) {
        *p++ = B64_CHARSET[(src[i] >> 2) & 0x3F];
        *p++ = B64_CHARSET[(src[i] << 4 | src[i+1] >> 4) & 0x3F];
        *p++ = B64_CHARSET[(src[i+1] << 2 | src[i+2] >> 6) & 0x3F];
        *p++ = B64_CHARSET[src[i+2] & 0x3F];
    }

    int padding = (length % 3)^3;

    if (padding == 1) {
        *p++ = B64_CHARSET[(src[i] >> 2) & 0x3F];
        *p++ = B64_CHARSET[(src[i] << 4 | src[i+1] >> 4) & 0x3F];
        *p++ = B64_CHARSET[(src[i+1] << 2) & 0x3F];
        *p++ = '=';
    } else if (padding == 2) {
        *p++ = B64_CHARSET[(src[i] >> 2) & 0x3F];
        *p++ = B64_CHARSET[(src[i] << 4) & 0x3F];
        *p++ = '=';
        *p++ = '=';
    }

    *p = '\0';

    return buffer;
}

char *io::b64_decode(const char *src, size_t length) {
    // The input is invalid or not properly padded
    if (length % 4 != 0)
        return nullptr;

    size_t buffer_length = (length * 3 >> 2) + 1;

    char *buffer = (char *)malloc((sizeof *buffer) * buffer_length);

    if (buffer == NULL) {
        printf("Call to malloc() in io::b64_decode() failed. Exiting...");
        exit(EXIT_FAILURE);
    }

    char *p = buffer;
    size_t i;

    for(i = 0; i < length - 4; i += 4) {
        *p++ = (B64_LOOKUP[(int)src[i]] << 2 | B64_LOOKUP[(int)src[i+1]] >> 4) & 0xff;
        *p++ = (B64_LOOKUP[(int)src[i+1]] << 4 | B64_LOOKUP[(int)src[i+2]] >> 2) & 0xff;
        *p++ = (B64_LOOKUP[(int)src[i+2]] << 6 | B64_LOOKUP[(int)src[i+3]]) & 0xff;
    }

    *p++ = (B64_LOOKUP[(int)src[i]] << 2 | B64_LOOKUP[(int)src[i+1]] >> 4) & 0xff;

    if (src[i+2] != '=') {
        *p++ = (B64_LOOKUP[(int)src[i+1]] << 4 | B64_LOOKUP[(int)src[i+2]] >> 2) & 0xff;
        if (src[i+3] != '=') {
            *p++ = (B64_LOOKUP[(int)src[i+2]] << 6 | B64_LOOKUP[(int)src[i+3]]) & 0xff;
        } else {
            *p++ = B64_LOOKUP[(int)src[i+2]] << 6 & 0xff;
        }
    } else {
        *p++ = B64_LOOKUP[(int)src[i+1]] << 4 & 0xff;
    }

    *p = '\0';

    printf("in: %lu | out %lu | decoded: %s\n", length, buffer_length, buffer);

    return buffer;
}

