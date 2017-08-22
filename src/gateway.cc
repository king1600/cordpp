#include "client.h"

using namespace cordpp;

const std::string get_os_name() {
  #ifdef _WIN32
    return "win32";
  #elif _WIN64
    return "win64";
  #elif __linux__
    return "linux";
  #elif __APPLE__ || __MACH__
    return "darwin";
  #elif __unix || __unix__
    return "unix";
  #elif __FreeBSD__
    return "freebsd";
  #else
    return "unknown";
  #endif
}

static std::map<uint16_t, std::string> DisconnectCodes = {
  {4000, "Unknown, something went wrong! Reconnect?"},
  {4001, "Invalid gateway opcode!"},
  {4002, "Invalid payload!"},
  {4003, "Payload sent before IDENTIFY or RESUME"},
  {4004, "Account token incorrent in IDENTIFY or RESUME"},
  {4005, "More than one IDETIFY or RESUME sent"},
  {4007, "Invalid sequence when RESUMING"},
  {4008, "Ratelimit your payloads!"},
  {4009, "Session timed out, reconnect."},
  {4010, "Invalid shard sent on IDENTIFY"},
  {4011, "This session will be overloaded, Please shard"}
};

Gateway::Gateway(Client *client,
  const std::string &host, uint8_t &id, uint8_t &max)
  : shard_id(id), max_shards(max), beat(client->service.get())
{
  is_resume = false;
  this->host_url = host; 
  this->client = client;
  conn = std::make_unique<WebsockClient>(client->service);
  connect(); 
}

void Gateway::reset() {
  // reset gateway connection internally
  conn.reset(new WebsockClient(client->service));
  connect();
}

void Gateway::send(GatewayOp op, const Json &data) {
  Json packet = {
    {"op", static_cast<uint16_t>(op)},
    {"d", data}
  };
  const std::string dumped = packet.dump();
  std::cout << "Sending: " << packet.dump(2) << std::endl;
  if (conn->state == WebsockState::open)
    conn->send(dumped.c_str(), dumped.size(), WebsockOp::text);
}

void Gateway::heartbeat() {
  // stop beating if websocket is not connected
  if (conn->state != WebsockState::open) {
    return;

  // no heartbeat ack (zombie conneciton), reconnect & resume
  } else if (!heartbeat_acked) {
    is_resume = true;
    conn->close(1011, "heartbeat stopped");

  // process heartbeat
  } else {
    Json sequence_data;
    if (sequence > 0)
      sequence_data = sequence;
    else
      sequence_data = nullptr;
    send(GatewayOp::Heartbeat, sequence_data);
    heartbeat_acked = false;
    beat.expires_from_now(boost::posix_time::millisec(heartbeat_interval));
    beat.async_wait([this](const Error &err) {
      if (!err) heartbeat();
    });
  }
}

void Gateway::identify() {
  Json data; // the data to send

  // prepare RESUME payload
  if (is_resume) {
    data = {
      {"token", client->token},
      {"session_id", session_id},
      {"seq", sequence}
    };

  // prepare IDENTIFY payload
  } else {
    data = {
      {"token", client->token},
      {"properties", {
        {"$os", get_os_name()},
        {"$browser", lib},
        {"$device", lib}
      }},
      {"compress", false},
      {"large_threshold", 250},
      {"shard", Json::array({shard_id, max_shards})},
      {"presence", {
        {"game", nullptr},
        {"status", "online"},
        {"since", nullptr},
        {"afk", false}
      }}
    };
  }

  // send payload
  send(is_resume ? GatewayOp::Resume : GatewayOp::Identify, data);
}

void Gateway::connect() {
  // reset heartbeat data on connection
  conn->on_connect([this](const Error &err) {
    beat.cancel();
    heartbeat_acked = false;
    heartbeat_interval = 0;

    // log connection
    std::cout << "[cordpp] Gateway Connected!" << std::endl;
  });

  // reconnect with optional delay if set
  conn->on_close([this](const WebsockCloseData &data) {
    
    // display closing information
    std::cerr << "[cordpp] Close Received -> "
      << "Code: " << data.code << " Reason: " << 
      (DisconnectCodes.find(data.code) != DisconnectCodes.end()
        ? DisconnectCodes[data.code] : data.reason) << std::endl;

    // reconnect with optional delay
    beat.cancel();
    if (resume_delay > 0) {
      beat.expires_from_now(boost::posix_time::millisec(resume_delay));
      resume_delay = 0;
      beat.async_wait([this](const Error &err) {
        if (!err) reset();
      });

    // reconnect immediately
    } else {
      reset();
    }
  });

  // handle incoming messages
  conn->on_message([this](const char *raw, const size_t raw_len) {
    // check if message data is valid json
    std::string raw_data(raw, raw_len);
    if (raw_data.front() != '{' || raw_data.back() != '}') {
      conn->close(4002, "Invalid payload sent");

    // convert message to json and decide what to do with it
    } else {
      Json data = Json::parse(raw_data);
      if (data.find("s") != data.end() && !data["s"].is_null())
        sequence = data["s"].get<uint64_t>();

      // debug log
      std::cout << data.dump(2) << std::endl;
      
      // choose action using the opcode from the packet
      switch (data["op"].get<GatewayOp>()) {
        case GatewayOp::Hello:
          heartbeat_acked = true;
          heartbeat_interval = data["d"]["heartbeat_interval"];
          heartbeat_interval -= 100; // room for error
          beat.expires_from_now(boost::posix_time::millisec(heartbeat_interval));
          beat.async_wait([this](const Error &err) { if (!err) heartbeat(); });
          identify();
          break;

        case GatewayOp::Dispatch:
          dispatch(data["t"], data["d"]);
          break;

        case GatewayOp::HeartbeatAck:
          std::cout << "[cordpp] Heartbeat Ack!" << std::endl;
          heartbeat_acked = true;
          break;

        case GatewayOp::Reconnect:
          is_resume = true;
          conn->close();
          break;

        case GatewayOp::RequestGuildMembers:
          break;

        case GatewayOp::VoiceStateUpdate:
          break;

        case GatewayOp::InvalidSession:
          is_resume = data["d"].get<bool>();
          resume_delay = 4000;
          conn->close();
          break;

        default: break;
      }
    }
  });

  // start connection
  conn->connect(host_url);
}