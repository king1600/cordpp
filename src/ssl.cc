#include "ssl.h"
#include <boost/bind.hpp>

using namespace cordpp;

Service::Service() : ssl_context(ssl::context::sslv23) {
  resolver = std::make_shared<tcp::resolver>(service);
}

void Service::run() {
  service.run();
}

tcp::resolver& Service::get_resolver() {
  return *(resolver.get());
}

asio::io_service& Service::get() {
  return service;
}

ssl::context& Service::get_ssl_context() {
  return ssl_context;
}

SSLClient::SSLClient(Service& service) :
  service(service), sock(service.get(), service.get_ssl_context()) {
  connected = false;                      // client starts of disconnected
  on_close([](const Error &e) {});        // default close callback
  sock.set_verify_mode(ssl::verify_none); // no ssl verification 
}

const bool SSLClient::is_connected() const {
  return connected;
}

void SSLClient::on_close(const BoostAction &action) {
  close_cb = action;
}

void SSLClient::write(const char *data, const size_t len) {
  if (!connected) return;
  asio::async_write(sock, asio::buffer(data, len),
    [this](const Error &e, size_t written) { if (e) close(e); });
}

void SSLClient::write(const Buffer& data) {
  if (!connected) return;
  write(&data[0], data.size());
}

void SSLClient::write(const std::string &data) {
  if (!connected) return;
  write(data.c_str(), data.size());
}

void SSLClient::read_handler(const Error &err, size_t bytes) {
  // close on error
  if (err) { close(err); return; }
  
  // get the action from the task queue
  const BufferAction action = std::move(queue.front());
  queue.pop_front();

  // give the action its requested data
  const char *data = asio::buffer_cast<const char*>(buffer.data());
  buffer.consume(bytes);
  action(Buffer(data, data + bytes));
}

void SSLClient::read(const BufferAction& action) {
  if (!connected) return;
  queue.push_back(action);
  
  // read at least {size} amount of data
  asio::async_read(sock, buffer, asio::transfer_at_least(1),
    boost::bind(&SSLClient::read_handler, this,
      asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void SSLClient::read(const unsigned int size, const BufferAction& action) {
  if (!connected) return;
  queue.push_back(action);

  // read at least {size} amount of data
  asio::async_read(sock, buffer, asio::transfer_exactly(size),
    boost::bind(&SSLClient::read_handler, this,
      asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void SSLClient::read_until(const std::string &term, const BufferAction& action) {
  if (!connected) return;
  queue.push_back(action);

  // read until {term} string
  asio::async_read_until(sock, buffer, term,
    boost::bind(&SSLClient::read_handler, this,
      asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void SSLClient::close(const Error &err) {
  // check connection status
  if (!connected) return;
  connected = false;

  // request cancel shutdown
  Error cancel_err;
  sock.lowest_layer().cancel(cancel_err);

  // shutdown read and write socket capabities + ssl teardown
  sock.async_shutdown([&](const Error &shutdown_err) {

    // close internal socket and perform callback
    Error place_holder;
    sock.lowest_layer().close(place_holder);
    close_cb(cancel_err ? cancel_err : shutdown_err);
  });
}

void SSLClient::connect(const std::string &host,
  const int port, const BoostAction &action)
{
  // already connected
  if (connected) return;

  // resolve the host
  tcp::resolver::query query(host, std::to_string(port));
  service.get_resolver().async_resolve(query, 
  [this, &action] (const Error &err, tcp::resolver::iterator it) {;
    if (err) { action(err); return; }

    // connect to the endpoint
    asio::async_connect(sock.lowest_layer(), it,
    [this, &action] (const Error &err, tcp::resolver::iterator it) {
      if (err) { action(err); return; }

      // perform ssl handshake
      sock.async_handshake(ssl::stream_base::client, [this, &action] (const Error &err) {
        if (err) {
          action(err);
        } else {
          connected = true;
          action(Success);
        }
      });
    });
  });
}