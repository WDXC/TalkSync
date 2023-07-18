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
  void Disconnect();
  void RunLoop();

private:
  static void read_cb(struct bufferevent *bev, void *ctx);
  static void event_cb(struct bufferevent *bev, short events, void *ctx);
  static void send_cb(int fd, short events, void* arg);

private:
  static event_base *base_;
  static bufferevent *bev_;
  static std::string msg;
  static struct event* timerEvent_;
};

#endif
