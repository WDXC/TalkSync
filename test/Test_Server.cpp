#include <EchoClient.h>
#include <EchoServer.h>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

void RunServer() {
  EchoServer server;
  server.Start();
  server.RunLoop();
}

void RunClient() {}

void test() {
  // 创建服务器对象
  EchoServer server;
  EchoClient cli;

  // 启动服务器线程
  std::future<void> serverFuture = std::async(std::launch::async, [&server]() {
    server.Start();
    server.RunLoop();
  });

  // 启动客户端线程
  std::future<void> clientFuture = std::async(std::launch::async, [&cli]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cli.Start();
    cli.RunLoop();

    std::cout << "111" << std::endl;
    cli.SendMessage("Hello");
    std::cout << "22" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string tmp = cli.GetMsg();
  });

  // 等待客户端线程完成
  clientFuture.get();

  cli.Disconnect();

  // 停止服务器
  server.Stop();

  // 等待服务器线程完成
  serverFuture.get();
}

int main(int argc, char **argv) {
  test();
  return 0;
}
