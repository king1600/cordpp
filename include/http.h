#pragma once

#include "ssl.h"
#include "json.h"
#include "utils.h"

namespace cordpp {

  using Json = nlohmann::json;
  typedef std::function<void(const Json&)> JsonAction;
  static const Json JsonEmpty = Json::parse("{}");
  static const JsonAction JsonActionEmpty = [](const Json& j){};

  struct Response {
    int status;
    bool gzipped;
    std::string body;
  };

  struct Request {
    Json body;
    std::string method;
    std::string endpoint;
    std::string audit_reason;
  };

  struct RestTask {
    Request req;
    JsonAction action;
  };

  struct RestRoute {
    bool limited;
    std::deque<RestTask> pending;
  };

  class RestClient {
  private:
    Service &service;
    std::string token;
    Response response;
    RestRoute global_route;
    std::deque<RestTask> tasks;
    std::unique_ptr<SSLClient> conn;
    std::vector<std::string> cookies;
    std::map<std::string, RestRoute> routes;

    void parse_data();

    void handle_response();

    void connect_and_flush();

    void perform_task(const RestTask &req);

    RestRoute& get_route(const Request &req);

    const std::string to_string(const Request &req);

  public:
    RestClient(Service &service);

    void set_token(const std::string &token);

    void request(const std::string &method, const std::string &endpoint,
      const std::string &audit, const Json &data = JsonEmpty,
      const JsonAction &action = JsonActionEmpty);
  };

}