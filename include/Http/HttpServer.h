#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <event2/event.h>
#include <event2/http.h>
#include <string>
#include <unordered_map>

class HttpServer {
public:
    HttpServer();
    ~HttpServer();
    bool Init(int port);
    void Start();

private:
    static void RequestHandler(struct evhttp_request* req, void* arg);
    static void SendResponse(struct evhttp_request* req, const std::string& response);
    static void LoginHandler(struct evhttp_request* req, void* arg);

    // 添加用户会话管理的成员变量和方法
    std::unordered_map<std::string, std::string> user_sessions_;

    struct event_base* base_;
    struct evhttp* http_server_;
};

#endif // HTTP_SERVER_H

