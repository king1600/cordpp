#pragma once

#include "http.h"
#include "gateway.h"

namespace cordpp {

  class Client {
  private:
    uint8_t shard_id;
    uint8_t max_shards;
    std::unique_ptr<RestClient> _api;
    std::unique_ptr<Gateway> _gateway;

  public:
    Service service;
    std::string token;

    inline RestClient& api() {
      return *(_api.get());
    }

    inline Gateway& gateway() {
      return *(_gateway.get());
    }

    Client(uint8_t id = 0, uint8_t max = 0);

    void login(const std::string &_token);
  };

}