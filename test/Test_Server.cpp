#include "HttpServer.h"
#include "gtest/gtest.h"
#include <Client.h>
#include <chrono>
#include <memory>
#include <thread>

using testing::InitGoogleTest;
using testing::Test;

class HttpServerTest : public Test {
public:
  static void SetUpTestSuite() {
    server = std::make_shared<HttpServer>();
    serverThread = std::thread([&]() { server->Start(); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (serverThread.joinable()) {
      printf("the server thread is started\n");
      cli = std::make_shared<Client>();
      clientThread = std::thread([&]() { cli->Connect("127.0.0.1", 9876); });
    }

    if (clientThread.joinable()) {
      printf("the client thread is started\n");
    }
  }
  static void TearDownTestSuite() {
    cli->Disconnect();
    if (clientThread.joinable()) {
      clientThread.join();
    }
    server->Stop();
    if (serverThread.joinable()) {
      serverThread.join();
    }
  }

protected:
  static std::shared_ptr<HttpServer> server;
  static std::shared_ptr<Client> cli;
  static std::thread serverThread;
  static std::thread clientThread;
};

std::shared_ptr<HttpServer> HttpServerTest::server;
std::shared_ptr<Client> HttpServerTest::cli;
std::thread HttpServerTest::serverThread;
std::thread HttpServerTest::clientThread;

TEST_F(HttpServerTest, TestBevEventEOF) {
  cli->SendData("Hello");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_STREQ("Hello", Client::GetMsg());
}

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  //    HttpServer ser;
  //    ser.Start();
}
