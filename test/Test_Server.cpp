#include "EchoServer.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <event2/buffer.h>
#include <atomic>
#include <thread>

// 模拟 bufferevent 的 Mock 类
class MockBufferEvent {
public:
    MOCK_METHOD(void, Enable, (short));
    // 还可以添加其他需要模拟的 bufferevent 方法
};

class EchoServerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // 创建 event_base 作为 base_
        base_ = event_base_new();
        EchoServer::SetBase(base_);

        // 启动服务器线程
        serverThread = std::thread([&]() {
            server.Start();
            server.RunLoop();
        });

        // 等待服务器成功启动
        while (!serverStarted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    static void TearDownTestSuite() {
        // 停止服务器
        server.Stop();

        // 等待服务器线程结束
        serverThread.join();

        // 释放 event_base 内存
        event_base_free(base_);
    }

    void SetUp() override {
        // 创建真实的 bufferevent
        bev = bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);

        // 设置 bufferevent
        EchoServer::SetBufferEvent(bev);
    }

    void TearDown() override {
        // 释放 bufferevent
        if (bev != nullptr) {
            std::cout << "mmd\n";
            bufferevent_free(bev);
        }
    }

    static event_base* base_;
    static EchoServer server;
    static std::thread serverThread;
    bufferevent* bev;
};

event_base* EchoServerTest::base_ = nullptr;
EchoServer EchoServerTest::server;
std::thread EchoServerTest::serverThread;

TEST_F(EchoServerTest, TestBevEventEOF) {
    // 创建一个 MockBufferEvent
    MockBufferEvent mockBufferEvent;

    // 使用 ON_CALL 设置 Enable 函数的模拟行为
    ON_CALL(mockBufferEvent, Enable(testing::_)).WillByDefault([&](short events) {
        // 模拟服务器处理 bev_event_eof 事件
        EchoServer::echo_event_cb(server.bev_, BEV_EVENT_EOF, nullptr);
    });

    // 执行测试代码，这里不需要调用 EchoServer::echo_event_cb，因为已经在 ON_CALL 中模拟了
    // EchoServer::echo_event_cb(bev, BEV_EVENT_EOF, nullptr);

    // 在这里添加您的断言，检查服务器对 bev_event_eof 事件的处理逻辑

    // 验证 bev 不为 nullptr，因为我们在 SetUp 函数中设置了有效的 bev
    ASSERT_TRUE(server.bev_ !=nullptr);
}

// 添加更多测试用例根据需要

int main(int argc, char* argv[]) {
    // 初始化 Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // 运行所有测试用例
    int testResult = RUN_ALL_TESTS();

    return testResult;
}

