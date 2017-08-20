#include "http.h"

using namespace cordpp;

int main() {
  Service loop;
  RestClient client(loop);
  client.set_token("");

  client.request("GET", "/gateway/bot", "", {}, [](const Json &data) {
    std::cout << "Handling response" << std::endl;
  });

  loop.run();
}