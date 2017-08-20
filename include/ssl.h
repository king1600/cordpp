#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>

namespace cordpp {

  namespace asio = boost::asio;
  namespace ssl  = asio::ssl;
  using tcp = asio::ip::tcp;

  class Service {
  private:
    ssl::context ssl_context;
    asio::io_service service;
  public:
    Service();

    asio::io_service& get();
    ssl::context& get_ssl();
  };

  class SSLClient {
  };

}