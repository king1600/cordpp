#include "base64.h"
#include <cstring>

static const char B64_CHARSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const unsigned char B64_LOOKUP[256] = {
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
  64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

char* cordpp::b64_encode(const char *src, const size_t length) {
  // (length * 4 / 3) rounded up to nearest multiple of 4
  const size_t encoded_length = ((length<<2)/3+3)&-4;
  char *buffer = new char[encoded_length + 1];

  // perform base64 calculation
  size_t i;
  char *p = buffer;
  for(i = 0; i < length-2; i += 3) {
      *p++ = B64_CHARSET[(src[i] >> 2) & 0x3F];
      *p++ = B64_CHARSET[((uint8_t)src[i] << 4 | (uint8_t)src[i+1] >> 4) & 0x3F];
      *p++ = B64_CHARSET[((uint8_t)src[i+1] << 2 | (uint8_t)src[i+2] >> 6) & 0x3F];
      *p++ = B64_CHARSET[(uint8_t)src[i+2] & 0x3F];
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

  // add null terminator and return constructed buffer
  *p = '\0';
  return buffer;
}

char* cordpp::b64_decode(const char *src) {
  // The input is invalid or not properly padded
  const size_t src_len = std::strlen(src);
  if (src_len % 4 != 0)
    return nullptr;

  // instead of figuring out exactly how long the decoded string should be
  // allocate enough. at most this is an extra 2 bytes
  char *buffer = new char[(src_len * 3 >> 2) + 1];

  // decode content with lookup table
  size_t i;
  char *p = buffer;
  for(i = 0; i < src_len - 4; i += 4) {
    *p++ = (B64_LOOKUP[(uint8_t)src[i]] << 2 | B64_LOOKUP[(uint8_t)src[i+1]] >> 4) & 0xff;
    *p++ = (B64_LOOKUP[(uint8_t)src[i+1]] << 4 | B64_LOOKUP[(uint8_t)src[i+2]] >> 2) & 0xff;
    *p++ = (B64_LOOKUP[(uint8_t)src[i+2]] << 6 | B64_LOOKUP[(uint8_t)src[i+3]]) & 0xff;
  }

  // decode padding
  *p++ = (B64_LOOKUP[(uint8_t)src[i]] << 2 | B64_LOOKUP[(uint8_t)src[i+1]] >> 4) & 0xff;

  // decode rest of padding
  if (src[i+2] != '=') {
    *p++ = (B64_LOOKUP[(uint8_t)src[i+1]] << 4 | B64_LOOKUP[(uint8_t)src[i+2]] >> 2) & 0xff;
    if (src[i+3] != '=')
      *p++ = (B64_LOOKUP[(uint8_t)src[i+2]] << 6 | B64_LOOKUP[(uint8_t)src[i+3]]) & 0xff;
    else
      *p++ = B64_LOOKUP[(uint8_t)src[i+2]] << 6 & 0xff;
  } else {
    *p++ = B64_LOOKUP[(uint8_t)src[i+1]] << 4 & 0xff;
  }

  // add null terminator and return constructed buffer
  *p = '\0';
  return buffer;
}