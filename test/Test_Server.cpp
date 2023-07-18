#include "EchoClient.h"
#include "EchoServer.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> ServerStarted(false);
std::atomic<bool> clientEnd(false);
std::atomic<bool> allend(false);

EchoClient cli;
EchoServer server;

void fun_server() {
  // 进行服务器的初始化和运行
  server.Start();
  ServerStarted.store(true);
  server.RunLoop();
  clientEnd.store(true);
}

void fun_client() {
  while (!ServerStarted.load()) {
    std::this_thread::yield();
  }

  // 执行客户端相关操作
  cli.Start();
  cli.RunLoop();

  std::string tmp = cli.GetMsg();

  server.Stop();
  clientEnd.store(true);
}

void test() {
  // 启动服务器线程
  std::thread serverThread(fun_server);

  // 启动客户端线程
  std::thread cli_thread(fun_client);

  serverThread.join();
  cli_thread.join();

  allend.store(true);
  std::cout << "main thread executable" << std::endl;
}

int main(int argc, char** argv) {
  test();
  return 0;
}
