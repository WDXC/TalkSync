#include "HttpServer.h"
#include "LoginImpl.h"
#include <event2/buffer.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

HttpServer::HttpServer() : base_(nullptr), http_server_(nullptr) {}

HttpServer::~HttpServer() {
    if (http_server_) {
        evhttp_free(http_server_);
    }
    if (base_) {
        event_base_free(base_);
    }
}

bool HttpServer::Init(int port) {
    base_ = event_base_new();
    if (!base_) {
        std::cerr << "Could not initialize libevent!" << std::endl;
        return false;
    }

    http_server_ = evhttp_new(base_);
    if (!http_server_) {
        std::cerr << "Could not create HTTP server!" << std::endl;
        return false;
    }

    evhttp_set_gencb(http_server_, RequestHandler, this);

    if (evhttp_bind_socket(http_server_, "0.0.0.0", port) != 0) {
        std::cerr << "Could not bind to port " << port << "!" << std::endl;
        return false;
    }

    return true;
}

void HttpServer::Start() {
    if (base_) {
        std::cout << "HTTP server started..." << std::endl;
        event_base_dispatch(base_);
    }
}

void HttpServer::RequestHandler(struct evhttp_request* req, void* arg) {
    if (req && arg) {
        HttpServer* server = static_cast<HttpServer*>(arg);
        const char* uri = evhttp_request_get_uri(req);

        if (strncmp(uri, "/login", 6) == 0) {
            // 处理登录请求
            LoginHandler(req, server);
        } else {
            // 处理其他请求
            // 这里可以加入其他路由和请求处理逻辑
            std::string response = "Hello, World!";
            SendResponse(req, response);
        }
    }
}

void HttpServer::SendResponse(struct evhttp_request* req, const std::string& response) {
    struct evbuffer* buffer = evhttp_request_get_output_buffer(req);
    if (buffer) {
        evbuffer_add_printf(buffer, "%s", response.c_str());
        evhttp_send_reply(req, HTTP_OK, "OK", buffer);
    }
}

void HttpServer::LoginHandler(struct evhttp_request* req, void* arg) {
    HttpServer* server = static_cast<HttpServer*>(arg);
    struct evbuffer* buffer = evhttp_request_get_input_buffer(req);

    // 读取登录请求的内容
    size_t len = evbuffer_get_length(buffer);
    char* request_data = new char[len + 1];
    evbuffer_copyout(buffer, request_data, len);
    request_data[len] = '\0';

    // 解析登录请求
    std::string request_string(request_data);
    delete[] request_data;

    // 从登录请求中提取用户名和密码
    std::string username, password;
    // 这里根据实际的请求格式从 request_string 中提取用户名和密码

    // 使用登录模块进行身份验证
    LoginModuleImpl login_module("/home/neil/mine/Project/C++/TalkSync/db/mydatabase.db");
    if (login_module.AuthenticateUser(username, password)) {
        // 生成登录成功响应
        std::string session_id = login_module.GenerateSessionId();
        server->user_sessions_[session_id] = username; // 存储用户会话信息
        std::string response = "Login successful. Session ID: " + session_id;
        SendResponse(req, response);
    } else {
        // 生成登录失败响应
        std::string response = "Login failed. Invalid username or password.";
        SendResponse(req, response);
    }
}
