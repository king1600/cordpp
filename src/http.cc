#include "http.h"
#include <ctime>
#include <sstream>

using namespace cordpp;

static const std::string CLRF = "\r\n";
static const std::string http_prefix = "HTTP/x.x ";

void RestClient::set_token(const std::string &token) {
  this->token = token;
}

void RestClient::get(const std::string &endpoint, const std::string &audit,
  const Json &data, const JsonAction &action) {
  request("GET", endpoint, audit, data, action);
}

void RestClient::post(const std::string &endpoint, const std::string &audit,
  const Json &data, const JsonAction &action) {
  request("POST", endpoint, audit, data, action);
}

void RestClient::put(const std::string &endpoint, const std::string &audit,
  const Json &data, const JsonAction &action) {
  request("PUT", endpoint, audit, data, action);  
}

void RestClient::del(const std::string &endpoint, const std::string &audit,
  const Json &data, const JsonAction &action) {
  request("DELETE", endpoint, audit, data, action);
}

RestRoute& RestClient::get_route(const Request &req) {
  const std::string index = req.method + ";" + req.endpoint;
  if (routes.find(index) == routes.end())
    routes[index] = RestRoute();
  return routes[index];
}

void RestClient::request(const std::string &method, const std::string &endpoint,
  const std::string &audit, const Json &data, const JsonAction &action)
{
  Request request{data, method, endpoint, audit};
  RestTask task{request, action};
  if (!conn->is_connected())
    tasks.push_back(task);
  else
    perform_task(task);
}

void RestClient::perform_task(const RestTask &task) {
  RestRoute& route = get_route(task.req);
  if (route.rate_limited) {
    route.pending.push_back(task);
  } else if (global_route.rate_limited) {
    global_route.pending.push_back(task);
  } else {
    conn->write(to_string(task.req));
    tasks.push_back(task);
  }
}

RestClient::RestClient(Service &s) : service(s) {
  global_route.rate_limited = false;
  conn = std::make_unique<SSLClient>(service);
  conn->on_close([this](const Error &err) {
    conn.reset(new SSLClient(service));
    connect_and_flush();
  });
  connect_and_flush();
}

void RestClient::connect_and_flush() {
  conn->connect(base_host, 443, [this](const Error &err) {
    if (err) { conn->close(err); return; }
    parse_data();
    RestTask task;
    const size_t pending = tasks.size();
    for (size_t i = 0; i < pending; i++) {
      task = std::move(tasks.front());
      tasks.pop_front();
      perform_task(task);
    }
  });
}

const std::string RestClient::to_string(const Request &req) {
  std::ostringstream output;

  // add method and endpoint
  output << req.method << " " << base_endpoint << req.endpoint << " HTTP/1.1\r\n";

  // add default headers
  output << "Accept: */*\r\n";
  //output << "Accept-Encoding: gzip\r\n";
  output << "Connection: keep-alive\r\n";
  output << "Host: " << base_host << ":443\r\n";
  output << "Authorization: Bot " << token << "\r\n";
  output << "User-Agent: DiscordBot (" << lib << ", " << version << ")\r\n";

  // add variable headers
  if (!req.audit_reason.empty())
    output << "X-Audit-Log-Reason: " << req.audit_reason << CLRF;
  if (!cookies.empty()) {
    output << "Cookie: ";
    for (const std::string &cookie : cookies)
      output << cookie << "; ";
    output << CLRF;
  }

  // add conditional body headers & create content
  if (!req.body.empty() && req.body.is_object()) {
    std::string content = req.body.dump();
    //output << "Content-Encoding: gzip\r\n";
    output << "Content-Type: application/json; charset=UTF-8\r\n";
    output << "Content-Length: " << std::to_string(content.size()) << CLRF;
    output << CLRF << content;
  } else output << CLRF;

  // return built http request
  return output.str();
}

void RestClient::flush_route(RestRoute &route) {
  RestTask task;
  route.rate_limited = false;
  size_t i, remaining = route.pending.size();
  for (i = 0; i < remaining; i++) {
    task = std::move(route.pending.front());
    route.pending.pop_front();
    perform_task(task);
  }
}

void RestClient::handle_response() {
  // if response.gzipped: response.body = ....
  const Json data = Json::parse(response.body);
  const RestTask task = std::move(tasks.front());
  std::map<std::string, std::string> &headers = response.headers;
  tasks.pop_front();

  // check if next request will be rate limited
  if (headers.find("X-RateLimit-Remaining") != headers.end()) {
    if (std::stoi(headers["X-RateLimit-Remaining"], nullptr, 10) < 1) {

      // get time to wait for rate limit
      long wait_time;
      if (headers.find("X-RateLimit-Reset") != headers.end()) {
        const long now = static_cast<const long>(std::time(nullptr));
        long reset = std::stol(headers["X-RateLimit-Reset"], nullptr, 10);
        wait_time = (reset - now) * 1000;
      } else if (headers.find("Retry-After") != headers.end()) {
        wait_time = std::stol(headers["Retry-After"], nullptr, 10);
      }

      // if global, limit global route
      if (data.find("global") != data.end()) {
        if (data["global"].get<std::string>() == "true") {
          global_route.rate_limited = true;
          service.call_later(wait_time, nullptr, [this](void *data) {
            flush_route(global_route);
          });
        }
      }

      // limit request route as well
      RestRoute &route = get_route(task.req);
      route.rate_limited = true;
      service.call_later(wait_time, &route, [this](void *data) {
        flush_route(*reinterpret_cast<RestRoute*>(data));
      });
    }
  }

  // start parsing again and perform callback with data
  parse_data();
  task.action(data);
}

void RestClient::parse_data() {
  response.headers.empty();
  if (conn->is_connected()) {

    // parse status
    conn->read_until(CLRF, [this](const Buffer &buf) {
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
          cookies.push_back(cookie.substr(0, cookie.find(';')));
        }

        // check if response is gzipped
        if (response.headers.find("Content-Encoding") != response.headers.end())
          response.gzipped = response.headers["Content-Encoding"].find("gzip")
            < response.headers["Content-Encoding"].size();

        // parse body for fixed size
        if (response.headers.find("Content-Length") != response.headers.end()) {
          const unsigned int size = static_cast<const unsigned int>(
            std::stoi(response.headers["Content-Length"], nullptr, 10));
          conn->read(size, [this](const Buffer &buf) {
            response.body = std::string(buf.begin(), buf.end());
            handle_response();
          });

        // parse body for chunked buffers
        } else if (response.headers.find("Transfer-Encoding") != response.headers.end()) {
          conn->read_until("0\r\n\r\n", [this](const Buffer &buf) {
            response.body = "";
            size_t size_pos = 0;
            unsigned int chunk_size = 0;
            std::string body(buf.begin(), buf.end());
            while (true) {
              size_pos = body.find(CLRF);
              chunk_size = static_cast<unsigned int>(std::stoi(
                body.substr(0, size_pos), nullptr, 16));
              if (chunk_size < 1) break;
              response.body += body.substr(size_pos + CLRF.size(), chunk_size);
              body = body.substr(size_pos + CLRF.size() + chunk_size + CLRF.size());
            }
            handle_response();
          });
        }
      });
    });
  }
}