#include "ssl.h"
#include <iostream>

using namespace cordpp;

int main() {
  Service loop;
  SSLClient client(loop);

  client.connect("www.google.com", 443, [&](const Error &err) {
    std::cout << "Connected" << std::endl;
    std::string req = "GET /humans.txt HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
    client.write(req);

    client.read_until("\r\n\r\n", [&client](const Buffer &data) {
      std::cout << "Got Headers: ";
      std::cout.write(&data[0], data.size());
      std::cout << std::endl << std::endl;

      client.read_until("0\r\n\r\n", [](const Buffer &data) {
        std::cout << "Got body: ";
        std::cout.write(&data[0], data.size());
        std::cout << std::endl;
      });
    });
  });

  loop.run();
}