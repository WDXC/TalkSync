#ifndef _ECHOSERVER_H_
#define _ECHOSERVER_H_

#include <arpa/inet.h>
#include <atomic>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <thread>

extern  std::atomic<bool> serverStarted;
extern  std::atomic<bool> serverStopped;

class EchoServer {
public:
  EchoServer();
  ~EchoServer();
  void Start();
  void Stop();
  void RunLoop();

public:
  static void SetBase(event_base *base);

  static void cb(int sock, short what, void *arg);

  static void SetBufferEvent(bufferevent* bev);

  static void accept_conn_cb(struct evconnlistener *listener,
                             evutil_socket_t fd, struct sockaddr *address,
                             int socklen, void *ctx);
  static void accept_error_cb(struct evconnlistener *listener, void *ctx);

  static void echo_read_cb(struct bufferevent *bev, void *ctx);
  static void echo_event_cb(struct bufferevent *bev, short events, void *ctx);

public:
  static bufferevent* bev_;
  static event_base *base_;


private:
  static int flag_;
  evconnlistener *listener_;
  static event *watchdog_event_;
};

#endif
