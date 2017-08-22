#include "client.h"

using namespace cordpp;

Gateway::Gateway(Client *client,
  const std::string &host, uint8_t &id, uint8_t &max)
  : shard_id(id), max_shards(max)
{
  this->host_url = host;
  this->client = client;
  conn = std::make_unique<WebsockClient>(client->service);
  conn->on_connect([this](const Error &err) {
    if (err)
      std::cout << "Failed to connect: " << err.message() << std::endl;
    std::cout << "Connected!" << std::endl;
  });
  conn->on_close([this](const WebsockCloseData &data) {
    std::cout << "Closed > Code: " << data.code 
    << " Reason: " << data.reason << std::endl;
  });
  conn->on_message([this](const std::string &data) {
    std::cout << "Received data: " << data << std::endl;
    conn->close(1011, "Stuff happened");
  });
  conn->connect(host_url);
}