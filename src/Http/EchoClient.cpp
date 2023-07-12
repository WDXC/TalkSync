#include "EchoClient.h"
#include <event2/util.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <string.h>
#include <string>
#include <unistd.h>

#ifdef __GNUC__
#define CHECK_FMT(a, b) __attribute__((format(printf, a, b)))
#else
#define CHECK_FMT(a, b)
#endif

static void error(const char *fmt, ...) CHECK_FMT(1, 2);
static void error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

EchoClient::EchoClient() {}

EchoClient::~EchoClient() {}

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
  base_ = event_base_new();
  if (!base_) {
    perror("event_base_new()");
    return;
  }

  bev_ = bufferevent_socket_new(base_, -1,
                               BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);

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
  bufferevent_setcb(bev_, read_cb, nullptr, event_cb, nullptr);
  if (bufferevent_enable(bev_, EV_READ)) {
    error("Cannot monitor EV_READ|EV_WRITE for client\n");
    return;
  }

  if (bufferevent_socket_connect(bev_, sa, ss_len)) {
    error("Connection failed\n");
    return;
  }

  SendMessage("Hello");
  if (!event_base_dispatch(base_)) {
      return;
  }
}

void EchoClient::SendMessage(std::string msg) {
    struct evbuffer *out = bufferevent_get_output(bev_);
    evbuffer_add(out, msg.c_str(), msg.size());
    evbuffer_write(out, bufferevent_getfd(bev_));
}

void EchoClient::read_cb(struct bufferevent *bev, void *arg) {
  struct evbuffer *in = bufferevent_get_input(bev);
  event_base* tmp_base = bufferevent_get_base(bev);
  size_t size = evbuffer_get_length(in);
  char* message = new char[size + 1];
  evbuffer_copyout(in, message, size);
  message[size] = '\0';
  std::cout << "The back message is : " << message << std::endl;
  evbuffer_drain(in, size);
  delete[] message;
  event_base_loopbreak(tmp_base);
  bufferevent_free(bev);
  bev = NULL;
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
