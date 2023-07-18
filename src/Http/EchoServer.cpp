#include "EchoServer.h"
#include <arpa/inet.h>
#include <atomic>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <thread>

std::atomic<bool> serverStarted(false);
std::atomic<bool> serverStopped(false);

bufferevent *EchoServer::bev_;
event_base *EchoServer::base_;
event* EchoServer::watchdog_event_;

void EchoServer::cb(int sock, short what, void *arg) {
  if (serverStopped) {
    bufferevent_disable(bev_, EV_WRITE | EV_READ);
    bufferevent_free(bev_);
    event_base_loopbreak(base_);
    event_free(watchdog_event_);
    bev_ = nullptr;
    std::cout << "end base\n";
  } else {
    std::cout << "end failed\n";
  }
}

EchoServer::EchoServer() {}

EchoServer::~EchoServer() {
  listener_ = nullptr;
  base_ = nullptr;
  bev_ = nullptr;
}

void EchoServer::RunLoop() { event_base_dispatch(base_); }

void EchoServer::Start() {
  struct sockaddr_in sin;
  int port = 9876;

  evthread_use_pthreads();
  base_ = event_base_new();
  if (!base_) {
    puts("Couldn't open event base");
    return;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(0);
  sin.sin_port = htons(port);

  listener_ = evconnlistener_new_bind(base_, accept_conn_cb, nullptr,
                                      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                      -1, (struct sockaddr *)&sin, sizeof(sin));
  struct timeval five_seconds = {2, 0};
  watchdog_event_ = event_new(base_, -1, EV_PERSIST, cb, base_);

  event_add(watchdog_event_, &five_seconds);
  if (!listener_) {
    perror("Couldn't create listener");
    return;
  }
  evconnlistener_set_error_cb(listener_, accept_error_cb);

  serverStarted = true;


  // 注册一个在程序退出时释放 event_base 内存的函数
  std::atexit([]() {
    if (base_) {
      event_base_loopbreak(base_); // 停止事件循环
      event_base_free(base_); // 释放 event_base 的内存
    }
  });

}

void EchoServer::Stop() {
  serverStopped = true;
  evconnlistener_free(listener_);
}

void EchoServer::echo_read_cb(struct bufferevent *bev, void *ctx) {
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);
  evbuffer_enable_locking(output, NULL);

  evbuffer_add_buffer(output, input);
}

void EchoServer::echo_event_cb(struct bufferevent *bev, short events,
                               void *ctx) {

  std::cout << __FUNCTION__ << " you enter this function\n";
  if (events == BEV_EVENT_TIMEOUT) {
    std::cout << __FUNCTION__ << " you enter this function\n";
  }
  if (events == BEV_EVENT_CONNECTED) {
    std::cout << __FUNCTION__ << " you enter this function\n";
  }
  if (events & BEV_EVENT_CONNECTED) {
    std::cout << "Client connected." << std::endl;
  }
  if (events & BEV_EVENT_ERROR) {
    perror("Error from bufferevent");
  }
  if (events & BEV_EVENT_EOF) {
    event_base *tmp_base = bufferevent_get_base(bev);
    event_base_loopbreak(tmp_base);
    bufferevent_free(bev);
  }
  if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    bufferevent_free(bev);
  }
}

void EchoServer::accept_conn_cb(struct evconnlistener *listener,
                                evutil_socket_t fd, struct sockaddr *address,
                                int socklen, void *ctx) {
  event_base *base = evconnlistener_get_base(listener);
  bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  std::cout << __FUNCTION__ << " aabc \n";
  bufferevent_setcb(bev, echo_read_cb, nullptr, echo_event_cb, nullptr);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
  bufferevent_event_cb o = nullptr;
  bufferevent_data_cb a = nullptr;
  bufferevent_getcb(bev, &a, nullptr, &o, nullptr);

  bev_ = bev;
  base_ = base;
}

void EchoServer::accept_error_cb(struct evconnlistener *listener, void *ctx) {
  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr,
          "Got an error %d (%s) on the listener. "
          "Shutting down.\n",
          err, evutil_socket_error_to_string(err));
  event_base_loopexit(base, nullptr);
}
