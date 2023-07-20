#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "HttpStruct.h"
#include <arpa/inet.h>
#include <condition_variable>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <memory>
#include <mutex>

class HttpServer {
public:
  HttpServer();
  ~HttpServer();
  void Start();
  void Stop();
  bool IsStarted();

private:
  static void accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
                        struct sockaddr *address, int socklen, void *ctx);
  static void event_cb(bufferevent *bev, short events, void *arg);
  static void read_cb(bufferevent *bev, void *ctx);
  static void watchdog_cb(int fd, short events, void *ctx);
  static void accept_error_cb(struct evconnlistener *listener, void *ctx);

private:
  static int flags_;

  static event *watchdog_;

  static struct event_base *base_;

private:
  struct evconnlistener *listener_;
};

#endif
