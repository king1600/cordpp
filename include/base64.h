#pragma once

#include <cstdint>
#include <cstddef>

namespace cordpp {

  char* b64_encode(const char *src, const size_t len);
  char* b64_decode(const char *src);
  
}