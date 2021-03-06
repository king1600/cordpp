#pragma once

#include "ssl.h"
#include <chrono>

namespace cordpp {

  enum WebsockState { open, closed, connecting };
  enum WebsockOp { cont = 0, text = 1, bin = 2 ,close = 8, ping = 9, pong = 10 };

  struct WebsockCloseData {
    uint16_t code;
    std::string reason;
  };

  struct WebsockFrame {
    unsigned fin : 1;
    unsigned rsv1 : 1;
    unsigned rsv2 : 1;
    unsigned rsv3 : 1;
    unsigned masked : 1;

    char *payload;
    uint8_t *mask;
    WebsockOp opcode;
    size_t payload_len;
  };

  typedef std::chrono::steady_clock websock_clock;
  typedef std::function<void(const long)> WebsockPingAction;
  typedef std::function<void(const char*, const size_t)> DataAction;
  typedef std::function<void(const WebsockCloseData&)> WebsockCloseAction;

  struct WebsockPing {
    WebsockPingAction action;
    websock_clock::time_point created;
  };

  class WebsockClient {
  private:
    Buffer builder;
    WebsockFrame frame;
    SSLClient connection;

    DataAction message_cb;
    BoostAction connect_cb;
    WebsockCloseAction close_cb;
    std::deque<WebsockPing> pings;

    void handle_frame();

    void parse_length();

    void parse_headers();

    void parse_payload();

    void perform_close();

    void start_handshake(const std::string &host);

  public:
    WebsockState state;
    WebsockClient(Service &service);

    void connect(const std::string &host);

    void ping(const WebsockPingAction &action);

    void send(const char *data, const size_t len, WebsockOp op);

    void close(const uint16_t code = 1000, const std::string &reason = "");

    inline void on_connect(const BoostAction &action) {
      connect_cb = action;
    }

    inline void on_message(const DataAction &action) {
      message_cb = action;
    }

    inline void on_close(const WebsockCloseAction &action) {
      close_cb = action;
    }
  };

}