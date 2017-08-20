#include "http.h"
#include <sstream>

using namespace cordpp;

static const std::string http_prefix = "HTTP/x.x ";

RestClient::RestClient(Service &service) : service(service) {
  connected = false;
  conn = std::make_unique<SSLClient>(service);
  conn.on_close([this](const Error &err) {
    conn.reset(new SSLClient(service));
    conn.connect(base_host, 443, [this](const Error &err) {
      if (err) { conn.close(err); return; }
      parse_data();
    });
  });
  conn.connect(base_host, 443, [this](const Error &err) {
    if (err) { conn.close(err); return; }
    parse_data();
  });
}

void RestClient::set_token(const std::string &token) {
  this->token = token;
}

void RestClient::parse_data() {
  response = Response();
  if (conn->is_connected()) {

    // parse status
    conn->read_until("\r\n", [this](const Buffer &buf) {
      std::string data(buf.begin(), buf.end());
      data = data.substr(http_prefix.size());
      std::string status = data.substr(0, data.find(" "));
      response.status = std::stoi(status, nullptr, 10);
      
      // parse headers
      conn->read_until("\r\n\r\n", [this](const Buffer &buf) {
        std::string header;
        std::istringstream stream(&buf[0]);
        while (std::getline(stream, header) && header != "\r")
          response.headers[header.substr(0, header.find(":"))] =
            header.substr(header.find(": ") + 2);

        // save any cookies
        if (response.headers.find("Set-Cookie") != response.headers.end()) {
          std::string cookie = response.headers["Set-Cookie"];
          cookie = cookie.substr(0, cookie.find(";"));
          const size_t splitter = cookie.find("=");
          cookies[cookie.substr(0, splitter)] = cookie.substr(splitter + 1);
        }

        // parse body for fixed size
        if (response.headers.find("Content-Length") != response.headers.end()) {

        // parse body for chunked sending
        } else if (response.headers.find("Transfer-Encoding") != response.headers.end()) {

        }
      });
    });
  }
}

void RestClient::request(const std::string &method, const std::string &endpoint,
  const std::string &audit, const Json &data, const JsonAction &action)
{
  
}