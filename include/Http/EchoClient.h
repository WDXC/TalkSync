#ifndef _ECHOCLIENT_H_
#define _ECHOCLIENT_H_

#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <string>

class EchoClient {
public:
  EchoClient();
  ~EchoClient();
  void Start();
  void SendMessage(std::string msg);
  std::string GetMsg();

private:
  static void read_cb(struct bufferevent *bev, void *ctx);
  static void event_cb(struct bufferevent *bev, short events, void *ctx);

private:
  event_base *base_;
  bufferevent *bev_;
  static std::string msg;
};

#endif
