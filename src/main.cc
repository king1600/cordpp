#include "http.h"

using namespace cordpp;

int main() {
  Service loop;
  RestClient client(loop);
  client.set_token("");

  client.request("GET", "/gateway/bot", "", {}, [&client](const Json &data) {
    std::cout << "Response: " << data.dump() << std::endl;

    client.request("GET", "/gateway/bot", "", {}, [&client](const Json &data) {
      std::cout << "Response2: " << data.dump() << std::endl;

      client.request("GET", "/gateway/bot", "", {}, [&client](const Json &data) {
        std::cout << "Response3: " << data.dump() << std::endl;

        client.request("GET", "/gateway/bot", "", {}, [&client](const Json &data) {
          std::cout << "Response4: " << data.dump() << std::endl;
        });
      });
    });
  });

  loop.run();
}