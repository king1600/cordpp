#include "ssl.h"

cordpp::Service::Service() : ssl_context(cordpp::ssl::context::sslv23) {

}

cordpp::asio::io_service& cordpp::Service::get() {
  return service;
}

cordpp::ssl::context& cordpp::Service::get_ssl() {
  return ssl_context;
}