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

void Service::call_later(const long delay, void *data, const RawAction &action)
{
  new Timer(service, delay, data, action, [](Timer *timer) {
    const RawAction &action = std::move(timer->get());
    action(timer->raw());
    delete timer;
  });
}

Timer::Timer(asio::io_service &service, const long delay_ms, void *data,
  const RawAction &action, const std::function<void(Timer*)> &cb) : timer(service)
{
  this->data = data;
  this->callback = cb;
  this->action = action;
  timer.expires_from_now(boost::posix_time::millisec(delay_ms));
  timer.async_wait([this](const Error &err) {
    if (!err) callback(this);
  });
}

SSLClient::SSLClient(Service& service) :
  service(service), sock(service.get(), service.get_ssl_context()) {
  remaining = 0;                          // no remaining read data
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

  // fix read_n incomplete size
  bytes += remaining;
  remaining = 0;

  // give the action its requested data
  const Buffer buf(
    asio::buffers_begin(buffer.data()),
    asio::buffers_begin(buffer.data()) + bytes);
  buffer.consume(bytes);
  action(std::move(buf));
}

void SSLClient::read(const BufferAction& action) {
  if (!connected) return;
  queue.push_back(action);
  
  // read some amount of data
  asio::async_read(sock, buffer, asio::transfer_at_least(1),
    boost::bind(&SSLClient::read_handler, this,
      asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void SSLClient::read(const unsigned int size, const BufferAction& action) {
  if (!connected) return;
  queue.push_back(action);

  // read at least {size} amount of data
  buffer.prepare(size);
  remaining = buffer.size();
  asio::async_read(sock, buffer,
    asio::transfer_exactly(size - remaining),
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
  connect_cb = action;

  // resolve the host
  tcp::resolver::query query(host, std::to_string(port));
  service.get_resolver().async_resolve(query, 
  [this] (const Error &err, tcp::resolver::iterator it) {;
    if (err) { connect_cb(err); return; }

    // connect to the endpoint
    asio::async_connect(sock.lowest_layer(), it,
    [this] (const Error &err, tcp::resolver::iterator it) {
      if (err) { connect_cb(err); return; }

      // perform ssl handshake
      sock.async_handshake(ssl::stream_base::client, [this] (const Error &err) {
        connected = err ? false : true;
        connect_cb(err ? err : Success);
      });
    });
  });
}