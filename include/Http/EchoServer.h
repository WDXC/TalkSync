#ifndef _ECHOSERVER_H_
#define _ECHOSERVER_H_

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>

class EchoServer {
public:
  EchoServer();
  ~EchoServer();
  void Start();
  void Stop();


private:
  static void accept_conn_cb(struct evconnlistener *listener,
                             evutil_socket_t fd, struct sockaddr *address,
                             int socklen, void *ctx);
  static void accept_error_cb(struct evconnlistener *listener, void *ctx);

  static void echo_read_cb(struct bufferevent *bev, void *ctx);
  static void echo_event_cb(struct bufferevent *bev, short events, void *ctx);

private:
  event_base* base_;
  evconnlistener* listener_;
};

#endif
