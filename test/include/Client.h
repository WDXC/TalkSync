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

class Client {
public:
  Client();
  ~Client();

  bool Connect(const std::string &serverAddress, int port);
  void Disconnect();
  bool SendData(const std::string &data);

  static const char *GetMsg();

private:
  static void event_cb(bufferevent *bev, short event, void *arg);
  static void read_cb(bufferevent *bev, void *ctx);
  static void watchdog_cb(int fd, short events, void* arg);

public:
  static std::string msg;
  static struct event_base *base_;
  static bufferevent *bev_;

  static std::mutex bevMutex_;
  static std::condition_variable clientStarted_;

  static event* watchdog_;
  static bool flag_;
};

#endif
