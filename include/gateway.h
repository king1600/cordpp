#pragma once

#include "ws.h"
#include "user.h"

namespace cordpp {

  enum GatewayOp : uint8_t {
    Dispatch            = 0,
    Heartbeat           = 1,
    Identify            = 2,
    StatusUpdate        = 3,
    VoiceStateUpdate    = 4,
    VoiceServerPing     = 5,
    Resume              = 6,
    Reconnect           = 7,
    RequestGuildMembers = 8,
    InvalidSession      = 9,
    Hello               = 10,
    HeartbeatAck        = 11
  };

  class Client;
  class Gateway {
  private:
    uint8_t &shard_id;
    uint8_t &max_shards;
    std::string host_url;
    std::unique_ptr<WebsockClient> conn;

    bool is_resume;
    uint64_t sequence;
    bool heartbeat_acked;
    uint32_t resume_delay;
    std::string session_id;
    asio::deadline_timer beat;
    uint32_t heartbeat_interval;

    void reset();

    void connect();

    void identify();

    void heartbeat();

    void send(GatewayOp op, const Json &data);

    void dispatch(const std::string &event, const Json &data);

  public:
    Client *client;
    Gateway(Client *client,
      const std::string &host, uint8_t &id, uint8_t &max);
  };

}