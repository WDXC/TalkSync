#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <string>
#include <memory>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include "ClientInterface.h"


class Client : public ClientInterface {
public:
    Client();
    ~Client();

    bool Connect(const std::string& serverAddress, int port) override;
    void Disconnect() override;
    bool SendData(const std::string& data) override;
    std::string GetMsg();

private:
    static void event_cb(bufferevent* bev, short event, void* arg);
    static void read_cb(bufferevent* bev, void* ctx);

public:
    static std::string msg;

private:
    struct event_base* base_;
    std::shared_ptr<bufferevent> bev_;
};

#endif
