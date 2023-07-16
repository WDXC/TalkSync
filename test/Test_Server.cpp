#include "EchoClient.h"
#include <pthread.h>
#include "EchoServer.h"
#include <gtest/gtest.h>

using namespace testing;

void* fun(void* arg) {
    EchoServer server;
    server.Start();
    return NULL;
}

TEST(EchoServerTest, EchoFunctionality) {

    // 启动服务器
    std::cout << "start server\n";
    pthread_t serverThread;
    int res = pthread_create(&serverThread, NULL, fun, NULL);
        if (res != 0) {
        printf("线程创建失败");
    }

    // 执行后续代码
    EchoClient cli;
    cli.Start();

    std::string tmp = cli.GetMsg();

    EXPECT_EQ("Hello", tmp);

//    pthread_join(serverThread, NULL);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
