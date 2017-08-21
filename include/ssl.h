#pragma once

#include <deque>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace cordpp {

  namespace asio = boost::asio;
  namespace ssl  = asio::ssl;
  using tcp = asio::ip::tcp;

  typedef std::vector<char> Buffer;
  typedef std::function<void()> Action;
  typedef boost::system::error_code Error;
  typedef std::function<void(void*)> RawAction;
  typedef std::function<void(const Error&)> BoostAction;
  typedef std::function<void(const Buffer&)> BufferAction;

  static const Error Success = 
  boost::system::errc::make_error_code(
    boost::system::errc::success);

  class Timer {
  private:
    void *data;
    RawAction action;
    asio::deadline_timer timer;
    std::function<void(Timer*)> callback;
  public:
    inline void* raw() { return data; }
    inline RawAction& get() { return action; }
    Timer(asio::io_service &service, const long delay_ms, void *data,
      const RawAction &action, const std::function<void(Timer*)> &cb);
  };

  class Service {
  private:
    ssl::context ssl_context;
    asio::io_service service;
    std::shared_ptr<tcp::resolver> resolver;

  public:
    Service();

    void run();
    asio::io_service& get();
    tcp::resolver& get_resolver();
    ssl::context& get_ssl_context();
    void call_later(const long delay, void *data, const RawAction &action);
  };

  class SSLClient {
  private:
    bool connected;
    Service& service;
    size_t remaining;
    BoostAction close_cb;
    BoostAction connect_cb;
    asio::streambuf buffer;
    ssl::stream<tcp::socket> sock;
    std::deque<BufferAction> queue;

    void read_handler(const Error &err, size_t bytes);

  public:
    SSLClient(Service& service);

    const bool is_connected() const;
    void on_close(const BoostAction &action);

    void write(const Buffer& data);
    void write(const std::string &data);
    void write(const char *data, const size_t len);

    void close(const Error &err = Success);
    void connect(const std::string &host,
      const int port, const BoostAction &action);

    void read(const BufferAction& cb);
    void read(const unsigned int size, const BufferAction& cb);
    void read_until(const std::string &term, const BufferAction& cb);
  };

}