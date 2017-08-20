#pragma once

namespace cordpp {

  typedef uint64_t snowflake;

  static const std::string lib = "cordpp";
  static const std::string ws_version = "6";
  static const std::string version = "0.1.0";
  static const std::string http_version = "7";
  static const std::string base_host = "discordapp.com";
  static const std::string base_endpoint = "/api/v" + http_version;

}