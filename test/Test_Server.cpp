#include <gmock/gmock.h>
#include "ClientInterface.h"

using namespace testing;

class MockClient : public ClientInterface {
public:
    MOCK_METHOD(bool, Connect, (const std::string& serverAddress, int port), (override));
    MOCK_METHOD(void, Disconnect, (), (override));
    MOCK_METHOD(bool, SendData, (const std::string& data), (override));
};

TEST(MyClientTest, SendDataSuccessfully) {
    MockClient mockClient;
    EXPECT_CALL(mockClient, Connect("server_address", 1234)).WillOnce(Return(true));
    EXPECT_CALL(mockClient, SendData("test_data")).WillOnce(Return(true));
    EXPECT_CALL(mockClient, Disconnect());

    // 在这里写测试代码，调用连接服务器和发送数据的函数，并验证其行为是否符合期望
}

