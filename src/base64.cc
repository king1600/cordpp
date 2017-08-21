#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <new>

#include "base64.h"

char *cordpp::b64_encode(const char *src, size_t length) {
    // (length * 4 / 3) rounded up to nearest multiple of 4
    size_t encoded_length = ((length<<2)/3+3)&-4;
    char *buffer = new char[encoded_length + 1];

    char *p = buffer;
    size_t i;

    for(i = 0; i < length-2; i += 3) {
        *p++ = B64_CHARSET[(src[i] >> 2) & 0x3F];
        *p++ = B64_CHARSET[(src[i] << 4 | src[i+1] >> 4) & 0x3F];
        *p++ = B64_CHARSET[(src[i+1] << 2 | src[i+2] >> 6) & 0x3F];
        *p++ = B64_CHARSET[src[i+2] & 0x3F];
    }

    // this returns 3 instead of 0 if we need no padding but
    // if we need no padding we dont need to do anything
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

char *cordpp::b64_decode(const char *src) {
    size_t src_len = strlen(src);

    // The input is invalid or not properly padded
    if (src_len % 4 != 0)
        return nullptr;

    // instead of figuring out exactly how long the decoded string should be
    // allocate enough. at most this is an extra 2 bytes
    char *buffer = new char[(src_len * 3 >> 2) + 1];

    char *p = buffer;
    size_t i;

    for(i = 0; i < src_len - 4; i += 4) {
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

    return buffer;
}
