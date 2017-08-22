#pragma once

#include "ws.h"

namespace cordpp {

  class Client;
  class Gateway {
  private:
    uint8_t &shard_id;
    uint8_t &max_shards;
    std::string host_url;
    uint32_t heartbeat_interval;
    std::unique_ptr<WebsockClient> conn;

  public:
    Client *client;
    Gateway(Client *client,
      const std::string &host, uint8_t &id, uint8_t &max);
  };

}