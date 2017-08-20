#include <cstdio>
#include <cstdlib>
#include <new>

#include "io/b64.h"

char *io::b64_encode(const char *src, size_t length) {
    // (length * 4 / 3) rounded up to nearest multiple of 4
    size_t encoded_length = ((length<<2)/3+3)&-4;
    char *buffer = (char*)malloc(sizeof(*buffer) * encoded_length + 1);

    if (buffer == NULL) {
        printf("Call to malloc() in io::b64_encode() failed. Exiting...");
        exit(EXIT_FAILURE);
    }

    char *p = buffer;

    size_t i;

    for(i = 0; i < length-2; i += 3) {
        *p++ = base64_charset[(src[i] >> 2) & 0x3F];
        *p++ = base64_charset[(src[i] << 4 | src[i+1] >> 4) & 0x3F];
        *p++ = base64_charset[(src[i+1] << 2 | src[i+2] >> 6) & 0x3F];
        *p++ = base64_charset[src[i+2] & 0x3F];
    }

    int padding = (length % 3)^3;

    if (padding == 1) {
        *p++ = base64_charset[(src[i] >> 2) & 0x3F];
        *p++ = base64_charset[(src[i] << 4 | src[i+1] >> 4) & 0x3F];
        *p++ = base64_charset[(src[i+1] << 2) & 0x3F];
        *p++ = '=';
    } else if (padding == 2) {
        *p++ = base64_charset[(src[i] >> 2) & 0x3F];
        *p++ = base64_charset[(src[i] << 4) & 0x3F];
        *p++ = '=';
        *p++ = '=';
    }

    *p = '\0';

    return buffer;
}

char *io::b64_decode(const char *src, size_t length);

/*
 * YW55IGNhcm5hbCBwbGVhc3Vy
 * YW55IGNhcm5hbCBwbGVhc3VyZS4=
 */