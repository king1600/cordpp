#include "client.h"

using namespace cordpp;

Client::Client(uint8_t id, uint8_t max) : service() {
  shard_id = id;
  max_shards = max;
  _api = std::make_unique<RestClient>(service);
}

void Client::login(const std::string &_token) {
  // set the bot token
  token = _token;
  _api->set_token(token);

  // fetch the gateway url
  _api->get("/gateway/bot", "", {}, [this](const Json &data) {
    max_shards = (max_shards < 1) ? data["shards"].get<uint8_t>() : max_shards;
    std::string url = data["url"].get<std::string>();
    url = url.substr(url.find("://") + 3);
    _gateway = std::make_unique<Gateway>(this, url, shard_id, max_shards);
  });

  // run the event loop
  service.run();
}