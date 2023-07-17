#include "EchoClient.h"
#include "EchoServer.h"
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mutex;
std::condition_variable cv;
bool serverStarted = false;
bool clientEnd = false;
bool allend = false;

EchoClient cli;
EchoServer server;

void *fun(void *arg) {
  // 进行服务器的初始化和运行
  server.Start();
  {
    std::unique_lock<std::mutex> lock(mutex);
    serverStarted = true;
    cv.notify_all(); // 通知等待的线程，服务器已启动
  }
  server.RunLoop();

  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [] { return clientEnd; });
    std::cout << "recv end" << std::endl;
  }

  {
    std::unique_lock<std::mutex> lock(mutex);
    allend = true;
    cv.notify_all(); // 通知等待的线程，服务器已结束
  }

  return NULL;
}

void *fun_cli(void *arg) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    // 等待服务器线程启动
    cv.wait(lock, [] { return serverStarted; });
  }

  // 执行客户端相关操作
  cli.Start();
  cli.RunLoop();

  std::string tmp = cli.GetMsg();


  server.Stop();
  std::cout << "stop" << std::endl;
  {
    std::unique_lock<std::mutex> lock(mutex);
    clientEnd = true;
    cv.notify_all(); // 通知等待的线程，客户端已结束
  }

  return NULL;
}

void test() {
  // 启动服务器线程
  pthread_t serverThread;
  int res = pthread_create(&serverThread, NULL, fun, NULL);
  if (res != 0) {
    printf("服务器线程创建失败\n");
  }

  // 启动客户端线程
  pthread_t cli_thread;
  res = pthread_create(&cli_thread, NULL, fun_cli, NULL);
  if (res != 0) {
    printf("客户端线程创建失败\n");
  }

  {
    std::unique_lock<std::mutex> lock(mutex);
    // 等待服务器线程结束
    cv.wait(lock, [] { return allend; });
  }
  std::cout << "main thread executable" << std::endl;

  pthread_join(cli_thread, NULL);
//  pthread_join(serverThread, NULL);
}

int main(int argc, char **argv) {
  test();
  return 0;
}
