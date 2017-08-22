#include "ws.h"
#include "utils.h"
#include "base64.h"
#include <random>
#include <sstream>

using namespace cordpp;

static std::random_device rdev;
static std::mt19937 rand_dev(rdev());
static std::uniform_int_distribution<int> rand_gen(1, 255);

static inline uint8_t rand_byte() {
  return static_cast<uint8_t>(rand_gen(rand_dev));
}

WebsockClient::WebsockClient(Service &service) 
  : connection(service)
{
  frame.payload = nullptr;
  state = WebsockState::closed;
  on_connect([](const Error &err) {});
  on_close([](const WebsockCloseData &data) {});
  on_message([](const char* data, const size_t len) {});
}

void WebsockClient::connect(const std::string &host) {
  start_handshake(host);
}

void WebsockClient::start_handshake(const std::string &host) {
  // connect to gateway
  state = WebsockState::connecting;
  frame.payload = new char[host.size() + 1];
  frame.payload[sizeof(frame.payload)] = 0;
  std::memcpy(frame.payload, host.c_str(), host.size());
  connection.connect(host, 443, [this](const Error &err) {

    // handle connection error
    if (err) {
      state = WebsockState::closed;
      connect_cb(err);
      return;
    }

    // generate websocket key
    uint8_t key_data[16] = { 0 };
    for (uint8_t i = 0; i < 16; i++)
      key_data[i] = rand_byte();
    char *key = b64_encode(reinterpret_cast<const char*>(key_data), 16);

    // generate http handshake data
    std::ostringstream output;
    output << "GET /?v=" << ws_version << "&encoding=json HTTP/1.1\r\n"
           << "Upgrade: WebSocket\r\n"
           << "Connection: Upgrade\r\n"
           << "Sec-WebSocket-Version: 13\r\n"
           << "Host: " << frame.payload << ":443\r\n"
           << "Sec-WebSocket-Key: " << key << "\r\n\r\n";
    connection.write(output.str());
    delete frame.payload;
    frame.payload = nullptr;
    delete key;

    // check handshake response and start connection
    connection.read_until("\r\n\r\n", [this](const Buffer &buf) {
      const std::string data(buf.begin(), buf.end());
      if (data.find("HTTP/1.1 101") == 0) {
        state = WebsockState::open;
        parse_headers();
        connect_cb(Success);
      }
    });
  });
}

void WebsockClient::parse_headers() {
  // clean leftover data
  if (frame.payload != nullptr) {
    delete frame.payload;
    frame.payload = nullptr;
  }

  // read and parse the first two bytes
  if (connection.is_connected()) {
    connection.read(2, [this](const Buffer &buf) {

      // parse the first byte
      frame.fin  = (buf[0] >> 7) & 1;
      frame.rsv1 = (buf[0] >> 6) & 1;
      frame.rsv2 = (buf[0] >> 5) & 1;
      frame.rsv3 = (buf[0] >> 4) & 1;
      frame.opcode = static_cast<WebsockOp>(buf[0] & 0x0f);

      // parse the second byte
      frame.payload_len = buf[1] & (~0x80);
      frame.masked = (buf[1] >> 7) & 1;

      // no mask, fetch the payload size
      if (frame.masked == 0) {
        frame.mask = nullptr;
        parse_length();

      // retrive mask then fetch payload size
      } else {
        frame.mask = new uint8_t[4];
        connection.read(4, [this](const Buffer &buf) {
          frame.mask[0] = static_cast<uint8_t>(buf[0]);
          frame.mask[1] = static_cast<uint8_t>(buf[1]);
          frame.mask[2] = static_cast<uint8_t>(buf[2]);
          frame.mask[3] = static_cast<uint8_t>(buf[3]);
          parse_length();
        });
      }
    });
  }
}

void WebsockClient::parse_length() {
  // start parsing body if no remaining data
  if (frame.payload_len != 0x7f && frame.payload_len != 0x7e) {
    parse_payload();

  // read remaining data of size if not
  } else {
    const uint8_t padding = frame.payload_len < 0x10000 ? 2 : 8;
    connection.read(padding, [this](const Buffer &buf) {
      size_t payload_size = 0;

      // 16-bit payload
      if (frame.payload_len == 0x7e) {
        payload_size |= static_cast<uint64_t>(buf[0] & 0xff) << 8;
        payload_size |= static_cast<uint64_t>(buf[1] & 0xff);

      // 64-bit payload
      } else if (frame.payload_len == 0x7f) {
        payload_size |= static_cast<uint64_t>(buf[0] & 0xff) << 56;
        payload_size |= static_cast<uint64_t>(buf[1] & 0xff) << 48;
        payload_size |= static_cast<uint64_t>(buf[2] & 0xff) << 40;
        payload_size |= static_cast<uint64_t>(buf[3] & 0xff) << 32;
        payload_size |= static_cast<uint64_t>(buf[4] & 0xff) << 24;
        payload_size |= static_cast<uint64_t>(buf[5] & 0xff) << 16;
        payload_size |= static_cast<uint64_t>(buf[6] & 0xff) << 8;
        payload_size |= static_cast<uint64_t>(buf[7] & 0xff);
      }

      // set size and parse payload
      frame.payload_len = payload_size;
      parse_payload();
    });
  }
}

void WebsockClient::parse_payload() {
  // prepare and read data
  frame.payload = new char[frame.payload_len];
  connection.read(frame.payload_len, [this](const Buffer &buf) {

    // get and unmask data from response
    for (size_t i = 0; i < buf.size(); i++)
      frame.payload[i] = (frame.masked == 0 ?
        buf[i] : buf[i] ^ frame.mask[i % 4]);

    // delete mask if frame was masked
    if (frame.masked == 1)
      delete frame.mask;

    // handle the colelcted frame
    handle_frame();
  });
}

void WebsockClient::handle_frame() {
  // prepend fragmented data if any
  if (frame.opcode != WebsockOp::cont && builder.size() > 0) {
    char *data = new char[builder.size() + frame.payload_len];
    std::memcpy(data, &builder[0], builder.size());
    std::memcpy(data + builder.size(), frame.payload, frame.payload_len);
    builder.clear();
    delete frame.payload;
    frame.payload = data;
    frame.payload_len = sizeof(frame.payload);
  }

  // handle what actiont to perform using the frame
  switch (frame.opcode) {

    // append to builder if payload is fragmented
    case WebsockOp::cont:
      builder.insert(builder.end(), frame.payload,
        frame.payload + frame.payload_len);
      break;

    // echo back ping from server if it sends
    case WebsockOp::ping:
      send(frame.payload, frame.payload_len, WebsockOp::pong);
      break;
    
    // handle ping response from server
    case WebsockOp::pong:
      if (!pings.empty()) {
        const WebsockPing task = std::move(pings.front());
        pings.pop_front();
        task.action(std::chrono::duration_cast<std::chrono::milliseconds>(
          websock_clock::now() - task.created).count());
      }
      break;

    // handle closing opcode from server
    case WebsockOp::close:
      if (state != WebsockState::closed) {
        send(frame.payload, frame.payload_len, WebsockOp::close);
        state = WebsockState::closed;
      }
      perform_close();
      break;

    // handle opcodes for binary and text
    default:
      message_cb(frame.payload, frame.payload_len);
      break;
  }

  // read the next frame
  parse_headers();
}

void WebsockClient::perform_close() {
  WebsockCloseData close_data;
  close_data.code = (frame.payload[0] & 0xff) << 8;
  close_data.code |= frame.payload[1] & 0xff;
  if (frame.payload_len > 2)
    close_data.reason = std::string(
      frame.payload + 2, frame.payload_len);
  connection.close(Success);
  close_cb(close_data);
}

void WebsockClient::ping(const WebsockPingAction &action) {
  const char data[2] = { 'a', 0 };
  send(data, 2, WebsockOp::ping);
  pings.push_back({ action, std::chrono::steady_clock::now() });
}

void WebsockClient::close(const uint16_t code, const std::string &reason) {
  if (!connection.is_connected()) return;
  char payload[2 + reason.size()];
  payload[0] = static_cast<char>((code >> 8) & 0xff);
  payload[1] = static_cast<char>(code & 0xff);
  std::memcpy(payload + 2, reason.c_str(), reason.size());
  send(payload, sizeof(payload), WebsockOp::close);
  state = WebsockState::closed;
}

void WebsockClient::send(const char *data, const size_t len, WebsockOp op) {
  // check if allowed to send data
  if (state != WebsockState::open || !connection.is_connected())
    return;
  
  // calculate payload size padding
  uint8_t padding;
  if (len < 0x7e)
    padding = 0;
  else if (len < 0x10000)
    padding = 2;
  else
    padding = 8;

  // create output buffer and incrementer
  uint64_t i, offset = 0;
  uint8_t output[6 + len + padding];

  // add the first byte header
  output[offset++] = 0x80 | static_cast<uint8_t>(op);

  // add the payload length
  if (padding == 0) {
    output[offset++] = 0x80 | len;
  } else if (padding == 2) {
    output[offset++] = 0x80 | 0x7e;
    output[offset++] = (len >> 8) & 0xff;
    output[offset++] = len & 0xff;
  } else if (padding == 8) {
    output[offset++] = 0x80 | 0x7f;
    output[offset++] = (len >> 56) & 0xff;
    output[offset++] = (len >> 48) & 0xff;
    output[offset++] = (len >> 40) & 0xff;
    output[offset++] = (len >> 32) & 0xff;
    output[offset++] = (len >> 24) & 0xff;
    output[offset++] = (len >> 16) & 0xff;
    output[offset++] = (len >> 8) & 0xff;
    output[offset++] = len & 0xff;
  }

  // add the mask and get its offset
  uint8_t mask_offset = offset;
  output[offset++] = rand_byte();
  output[offset++] = rand_byte();
  output[offset++] = rand_byte();
  output[offset++] = rand_byte();

  // add payload data and mask it
  for (i = 0; i < len; i++)
    output[offset++] = static_cast<uint8_t>(data[i])
      ^ output[(i % 4) + mask_offset];

  // write data to connection
  connection.write(reinterpret_cast<const char*>(output), sizeof(output));
}