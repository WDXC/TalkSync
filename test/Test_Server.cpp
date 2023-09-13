#include "HttpServer.h"
//#include "gtest/gtest.h"
//#include <Client.h>
//#include <chrono>
//#include <memory>
//#include <thread>
//
//using testing::InitGoogleTest;
//using testing::Test;
//
//class HttpServerTest : public Test {
//public:
//  static void SetUpTestSuite() {
//    server = std::make_shared<HttpServer>();
//    serverThread = std::thread([&]() { server->Start(); });
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    if (serverThread.joinable()) {
//      cli = std::make_shared<Client>();
//      clientThread = std::thread([&]() { cli->Connect("http://127.0.0.1:9876/"); });
//    }
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//
//  }
//  static void TearDownTestSuite() {
//    server->Stop();
//    cli->Disconnect();
//    if (clientThread.joinable()) {
//      clientThread.join();
//    }
//    if (serverThread.joinable()) {
//      serverThread.detach();
//    }
//  }
//
//protected:
//  static std::shared_ptr<HttpServer> server;
//  static std::shared_ptr<Client> cli;
//  static std::thread serverThread;
//  static std::thread clientThread;
//};
//
//std::shared_ptr<HttpServer> HttpServerTest::server;
//std::shared_ptr<Client> HttpServerTest::cli;
//std::thread HttpServerTest::serverThread;
//std::thread HttpServerTest::clientThread;
//
//// basic Echoserver test
//TEST_F(HttpServerTest, TestBevEventEOF) {
//
//  GTEST_SKIP();
//  cli->SendData("Hello");
//  std::this_thread::sleep_for(std::chrono::seconds(1));
//  EXPECT_STREQ("Hello", Client::GetMsg());
//}
//
///* Common Request Test*/
//// Test Request method
//// Test Request header
//// Test Request param
//// Test Request body
//// Test Response Code
//// Test Response header 
//// Test Response Body
//// Cookie and Session Test
//// Request Redirect Test
//// Cache Test
//// high concurrency Test
//// Stress Test
//
//TEST_F(HttpServerTest, TestGetRequest) {
//    // Arrange
//    std::string url = "http://example.com/api/data";
//    std::string expectedResponse = "Hello, World!";
//
//    // Act
//    std::string response = cli->SendData(url);
//    // Assert
//    EXPECT_EQ(response, expectedResponse);
//}



/* Timeout */
// Connect timeout
// Read timeout
// Write timeout
// Handle timeout request
// High concurrency request timeout
// Resuable connection Test
// timeout exception recovery
// timeout setting test

/* Malicious request */
// Invalid symbol in Request
// Request too long
// traverse path
// SQL injection
// XSS attack
// CSRF attack
// malicious File Upload
// lack of valid and filter
// malicious Request

/* URL path and Route */
// match path Test
// url Route Test
// url param Test
// path param Test
// static asset Test
// the proprity of Route
// Error handle Test
// Route param verify Test

/* Request boundary */
// The size of Requst boundary Test
// The size of Response boundary Test
// request boundary test
// response boundary test
// boundary exchange test

/* Redirect Test */
// Redirect status code test
// redirect aim test
// multistage Redirect test
// pass param test
// Redirect loop test

/* Thread */
// Thread Safe test
// concurrency request test
// resource manage test
// request order test
// race condition test
// thread pool test
// concurrency limit test
// multithread log test
// shared resource test
// error handle test

/* Session test */
// Session tag test
// session data store
// timeout and expired session
// session trace test
// session relase and destory test
// session shared test
// session security test
// concurrency session test
// session status recovery test
// session intercepted test

/* Security */
// input verify
// verify and grant authorizthor
// CSRF attack
// CORS attack
// sensitive info protected
// file upload test
// exception handle
// log handle 

/* Stress */
// The count of concurrency
// the count of request
// test the low time in high stress 
// resource stress test
// continuous stress test
// load balancing test
// long time load

//int main(int argc, char **argv) {
//  InitGoogleTest(&argc, argv);
//  return RUN_ALL_TESTS();
//}
int main(int argc, char** argv) {
    HttpServer* test = new HttpServer();
    test->Start();
}
