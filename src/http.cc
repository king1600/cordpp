#include "http.h"
#include <sstream>

using namespace cordpp;

static const std::string http_prefix = "HTTP/x.x ";

void RestClient::set_token(const std::string &token) {
  this->token = token;
}

void RestClient::perform_task(const RestTask &task) {
  std::cout << "Performing" << std::endl;
  RestRoute& route = get_route(task.req);
  if (route.limited) {
    std::cout << "Rate limited" << std::endl;
    route.pending.push_back(task);
  } else {
    std::cout << "Not rate limited" << std::endl;
    std::string data = to_string(task.req);
    std::cout << data << std::endl;
    conn->write(data);
    tasks.push_back(task);
  }
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

RestClient::RestClient(Service &s) : service(s) {
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
    output << "X-Audit-Log-Reason: " << req.audit_reason << "\r\n";
  if (!cookies.empty()) {
    output << "Cookie: ";
    for (const std::string &cookie : cookies)
      output << cookie << "; ";
    output << "\r\n";
  }

  // add conditional body headers & create content
  if (!req.body.empty() && req.body.is_object()) {
    std::string content = req.body.dump();
    //output << "Content-Encoding: gzip\r\n";
    output << "Content-Type: application/json; charset=UTF-8\r\n";
    output << "Content-Length: " << std::to_string(content.size()) << "\r\n";
    output << "\r\n" << content;
  } else output << "\r\n";

  // return built http request
  return output.str();
}

void RestClient::handle_response() {
  // do all the stuff from parse_in_background() after parse_response()
  std::cout << "Response: " << response.body << std::endl;
}

void RestClient::parse_data() {
  response = Response();
  std::cout << conn->is_connected() << std::endl;
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
        std::map<std::string, std::string> headers;
        while (std::getline(stream, header) && header != "\r")
          headers[header.substr(0, header.find(":"))] =
            header.substr(header.find(": ") + 2);

        // save any cookies
        if (headers.find("Set-Cookie") != headers.end()) {
          std::string cookie = headers["Set-Cookie"];
          cookies.push_back(cookie.substr(0, cookie.find(';')));
        }

        for (auto const &entry : headers)
          std::cout << entry.first << ": " << entry.second << std::endl;

        // check if response is gzipped
        if (headers.find("Content-Encoding") != headers.end())
          response.gzipped = headers["Content-Encoding"].find("gzip")
            < headers["Content-Encoding"].size();

        // parse body for fixed size
        if (headers.find("Content-Length") != headers.end()) {
          const unsigned int size = static_cast<const unsigned int>(
            std::stoi(headers["Content-Length"], nullptr, 10));
          std::cout << "Reading: " << size << std::endl;
          conn->read(size, [this](const Buffer &buf) {
            response.body = std::string(buf.begin(), buf.end());
            handle_response();
          });

        // parse body for chunked buffers
        } else if (headers.find("Transfer-Encoding") != headers.end()) {
          conn->read_until("0\r\n", [this](const Buffer &buf) {
            response.body = std::string(buf.begin(), buf.end());
            handle_response();
          });
        }
      });
    });
  }
}