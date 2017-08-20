#pragma once

namespace io {
    static const char base64_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *b64_encode(const char *src, size_t length);
    char *b64_decode(const char *src, size_t length);
}