#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "EchoServer.h"

class EchoServerTest : public testing::Test {
public:
    static void SetUpTestSuite() {
        server = std::make_shared<EchoServer>();
        serverThread = std::thread([&] {
            server->Start();
            server->RunLoop();
        });
        // Wait for the server to start
        while (!serverStarted) {}
    }

    static void TearDownTestSuite() {
        server->Stop();
        serverThread.join();
    }

    void SetUp() override {
        // Set up the connection to the server
        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(9876);
        inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);

        base = event_base_new();
        bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        ASSERT_NE(bev, nullptr);

        int res = bufferevent_socket_connect(bev, (struct sockaddr*)&sin, sizeof(sin));
        ASSERT_EQ(res, 0);
    }

    void TearDown() override {
        if (bev) {
            bufferevent_free(bev);
            bev = nullptr;
        }
        if (base) {
            event_base_free(base);
            base = nullptr;
        }
    }

protected:
    static std::shared_ptr<EchoServer> server;
    static std::thread serverThread;
    struct bufferevent* bev;
    event_base* base;
};

std::shared_ptr<EchoServer> EchoServerTest::server = nullptr;
std::thread EchoServerTest::serverThread;

TEST_F(EchoServerTest, TestBevEventEOF) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_NE(EchoServer::GetBufferEvent(), nullptr);

    // Simulate receiving EOF event from the server
    EchoServer::echo_event_cb(EchoServer::GetBufferEvent().get(), BEV_EVENT_EOF, nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_EQ(EchoServer::GetBufferEvent().get(), nullptr);
}

// Add more tests as needed

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

