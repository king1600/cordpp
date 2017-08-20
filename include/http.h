#pragma once

#include "ssl.h"
#include "json.h"
#include "utils.h"

namespace cordpp {

  using Json = nlohmann::json;

  class Request {
  public:
    std::string body;
    std::string method;
    std::string endpoint;
    std::string audit_reason;
  };

  class Response {
  public:
    Json body;
    int status;
    std::map<std::string, std::string> headers;
  };

  class Route {
  public:
    bool limited;
    std::deque<JsonAction> queue;
  };

  static const Json JsonEmpty = Json::parse("{}");
  typedef std::function<void(const Json&)> JsonAction;
  typedef std::function<void(const Response&)> ResponseAction;
  static const JsonAction JsonActionEmpty = [](const Json& j){};

  class RestClient {
  private:
    Service &service;
    std::string token;
    Response response;
    Route global_route;
    std::unique_ptr<SSLClient> conn;
    std::deque<ResponseAction> queue;
    std::vector<std::string> cookies;
    std::map<std::string, Route> routes;

    void parse_data();

  public:
    RestClient(Service &service);

    void set_token(const std::string &token);

    void request(const std::string &method, const std::string &endpoint,
      const std::string &audit, const Json &data = JsonEmpty,
      const JsonAction &action = JsonActionEmpty);
  };

}