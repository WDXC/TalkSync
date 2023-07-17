#include "EchoClient.h"
#include <chrono>
#include <event2/thread.h>
#include <event2/util.h>
#include <iostream>
#include <string.h>
#include <string>
#include <thread>
#include <unistd.h>

#ifdef __GNUC__
#define CHECK_FMT(a, b) __attribute__((format(printf, a, b)))
#else
#define CHECK_FMT(a, b)
#endif

struct event *EchoClient::timerEvent_;

struct bufferevent *EchoClient::bev_;

void EchoClient::send_cb(int fd, short events, void *arg) {
  std::cout << "Sending message..." << std::endl;

  // 创建消息内容
  const char *message = "Hello";

  // 发送消息给服务器
  bufferevent_write(bev_, message, strlen(message));

  // 重新设置定时器，实现定期发送消息
  timeval tv;
  tv.tv_sec = 1; // 定时器间隔为1秒
  tv.tv_usec = 0;
  event_add(timerEvent_, &tv);
}

std::string EchoClient::msg = "";

static void error(const char *fmt, ...) CHECK_FMT(1, 2);
static void error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

EchoClient::EchoClient() {}

EchoClient::~EchoClient() {
  bev_ = NULL;
  base_ = NULL;
}

void EchoClient::Disconnect() {
  if (bev_) {
    event_free(timerEvent_);
    bufferevent_disable(bev_, EV_READ | EV_WRITE); // 先禁用读写事件
    bufferevent_setcb(bev_, nullptr, nullptr, nullptr, nullptr); // 清除回调函数
    event_base *base = bufferevent_get_base(bev_); // 获取所属的 event_base
    bufferevent_setfd(bev_, -1); // 从 bufferevent 中移除底层文件描述符
    bufferevent_free(bev_);
    bev_ = nullptr;
    event_base_loopbreak(base); // 停止事件循环
  }
  if (base_) {
    event_base_free(base_);
    base_ = nullptr;
  }
}

void EchoClient::RunLoop() { event_base_loop(base_, EVLOOP_NO_EXIT_ON_EMPTY); }

ev_socklen_t make_address(struct sockaddr_storage *ss, const char *address,
                          ev_uint16_t port) {
  struct evutil_addrinfo *ai = NULL;
  struct evutil_addrinfo hints;
  char strport[NI_MAXSERV];
  int ai_result;
  ev_socklen_t ret = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = EVUTIL_AI_PASSIVE | EVUTIL_AI_ADDRCONFIG;
  evutil_snprintf(strport, sizeof(strport), "%d", port);
  if ((ai_result = evutil_getaddrinfo(address, strport, &hints, &ai)) != 0) {
    return 0;
  }
  if (!ai) {
    return 0;
  }
  if (ai->ai_addrlen > sizeof(*ss)) {
    evutil_freeaddrinfo(ai);
    return 0;
  }
  memcpy(ss, ai->ai_addr, ai->ai_addrlen);
  ret = (ev_socklen_t)ai->ai_addrlen;
  evutil_freeaddrinfo(ai);
  return ret;
}

void EchoClient::Start() {
  evthread_use_pthreads();
  base_ = event_base_new();
  if (!base_) {
    perror("event_base_new()");
    return;
  }

  bev_ = bufferevent_socket_new(
      base_, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);

  struct sockaddr_storage ss;
  struct sockaddr *sa = (struct sockaddr *)&ss;
  ev_socklen_t ss_len;
  const char *ip = "127.0.0.1";
  int port = 9876;
  ss_len = make_address(&ss, ip, port);
  if (!ss_len) {
    error("Cannot make address from %s:%d\n", ip, port);
    return;
  }

  // 创建定时器事件（timer event）
  timeval tv;
  tv.tv_sec = 1; // 初始定时器间隔为1秒
  tv.tv_usec = 0;
  timerEvent_ = event_new(base_, -1, EV_PERSIST, send_cb, nullptr);
  event_add(timerEvent_, &tv);
  bufferevent_setcb(bev_, read_cb, nullptr, event_cb, nullptr);
  if (bufferevent_enable(bev_, EV_READ)) {
    error("Cannot monitor EV_READ|EV_WRITE for client\n");
    return;
  }

  if (bufferevent_socket_connect(bev_, sa, ss_len)) {
    error("Connection failed\n");
    return;
  }
}

void EchoClient::SendMessage(std::string msg) {
  std::cout << "heere" << std::endl;
  struct evbuffer *out = bufferevent_get_output(bev_);
  evbuffer_add(out, msg.c_str(), msg.size());
  evbuffer_write(out, bufferevent_getfd(bev_));
}

std::string EchoClient::GetMsg() { return msg; }

void EchoClient::read_cb(struct bufferevent *bev, void *arg) {
  struct evbuffer *in = bufferevent_get_input(bev);
  event_base *tmp_base = bufferevent_get_base(bev);
  size_t size = evbuffer_get_length(in);
  char *message = new char[size + 1];
  evbuffer_copyout(in, message, size);
  message[size] = '\0';
  std::cout << "message : " << message << std::endl;
  msg += message;
  evbuffer_drain(in, size);
  delete message;
  if (bev) {
    event_free(timerEvent_);
    bufferevent_disable(bev, EV_READ | EV_WRITE); // 先禁用读写事件
    bufferevent_setcb(bev, nullptr, nullptr, nullptr, nullptr); // 清除回调函数
    event_base *base = bufferevent_get_base(bev); // 获取所属的 event_base
    bufferevent_setfd(bev, -1); // 从 bufferevent 中移除底层文件描述符
    bufferevent_free(bev);
    bev = nullptr;
    event_base_loopbreak(tmp_base); // 停止事件循环
    event_base_loopexit(tmp_base, NULL); 
  }
  if (tmp_base) {
    event_base_free(tmp_base);
    tmp_base = nullptr;
  }
}

void EchoClient::event_cb(struct bufferevent *bev, short events, void *ctx) {
  if (events & BEV_EVENT_CONNECTED) {
    std::cout << "Connected to server." << std::endl;
  } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
    if (events & BEV_EVENT_ERROR) {
      std::cerr << "Error" << std::endl;
    }
    std::cout << "Connection closed." << std::endl;
    bufferevent_free(bev);
    exit(1);
  }
}
