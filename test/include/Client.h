#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <condition_variable>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/util.h>
#include <memory>
#include <mutex>
#include <string>

enum METHOD_TYPE {
  GET = 1,
  POST = 2,
  PUT,
  OPTIONS,
  HEAD,
  DELETE,
  CONNECT,
  TRACE,
};

struct Request {
  METHOD_TYPE req;
  std::string userAgent;
  std::string Host;
};

class Client {
public:
  Client();
  ~Client();

  // Init and Connect Server
  bool Connect(const std::string &serverAddress);
  // Disconnect Server and clear resource
  void Disconnect();
  // Send data to server
  std::string SendData(const std::string &data);

  // read static data from read_cb
  static const char *GetMsg();

private:
  // event callbcak for bufferevent
  static void event_cb(bufferevent *bev, short event, void *arg);
  // read callback for bufferevent
  static void read_cb(bufferevent *bev, void *ctx);
  // clean resource per one second
  static void watchdog_cb(int fd, short events, void *arg);
  static void http_request_done(struct evhttp_request* req, void* ctx);

public:
  static std::string msg;
  static struct event_base *base_;
  static bufferevent *bev_;
  static event *watchdog_;
  static bool flag_;

  // Wait Thread start
  static std::mutex bevMutex_;
  // notify other thread current client have started 
  static std::condition_variable clientStarted_;
};

#endif
