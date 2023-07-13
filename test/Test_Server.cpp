#include "EchoClient.h"
#include <thread>
#include "EchoServer.h"
#include <gtest/gtest.h>

using namespace testing;


TEST(EchoServerTest, EchoFunctionality) {
    int port = 8888;
    EchoServer server;

    // 启动服务器
    std::thread serverThread([&server]() {
        server.Start();
    });

    // 等待服务器启动完成
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 执行后续代码
    EchoClient cli;
    cli.Start();

    std::string res = cli.GetMsg();

    EXPECT_EQ("Hello", res);

    // 停止服务器
    server.Stop();

    // 等待服务器线程退出
    serverThread.join();

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
