#include "HttpServer.h"
#include "event2/bufferevent.h"
#include "event2/event.h"

#include <bits/types/struct_timeval.h>
#include <event2/thread.h>
#include <memory>
#include <string.h>
#include <string>

struct event_base *HttpServer::base_;

// 自定义删除器函数
void customDeleter(event* ptr) {
    event_free(ptr);
}

HttpServer::HttpServer() : listener_(nullptr) {}

HttpServer::~HttpServer() {
  if (base_) {
    event_base_free(base_);
  }
}

void HttpServer::watchdog(evutil_socket_t fd, short event, void* ctx) {
    if (!base_) {
        event_base_loopbreak(base_);
    }
}

void HttpServer::Start() {

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

  listener_ = evconnlistener_new_bind(base_, accept_cb, nullptr,
                                      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                      -1, (struct sockaddr *)&sin, sizeof(sin));
  if (!listener_) {
    perror("Couldn't create listener");
    return;
  }

  timeval ct {8, 0};
  // 使用带有自定义删除器的 std::shared_ptr 构造函数
  m_closetimer = std::shared_ptr<event>(event_new(base_, -1, EV_PERSIST, watchdog, nullptr), customDeleter);

//  m_closetimer = std::shared_ptr<event>(event_new(base_, -1, EV_PERSIST, watchdog, nullptr));
  event_add(m_closetimer.get(), &ct);
  
  evconnlistener_set_error_cb(listener_, accept_error_cb);

  event_base_dispatch(base_);
}

void HttpServer::Stop() {
  if (listener_) {
    evconnlistener_free(listener_);
    listener_ = nullptr; // Reset the pointer to avoid double-free
  }
}

void HttpServer::accept_cb(struct evconnlistener *listener, evutil_socket_t fd,
                           struct sockaddr *sa, int socklen, void *ctx) {
  event_base *base = evconnlistener_get_base(listener);
  bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, read_cb, nullptr, event_cb, nullptr);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}


void HttpServer::event_cb(bufferevent *bev, short events, void *arg) {
  if (events & BEV_EVENT_ERROR) {
    printf(" events\n");
    event_base *tmp_base = bufferevent_get_base(bev);
    event_base_loopbreak(tmp_base);
    bufferevent_free(bev);

    return;
  }
  if (events & BEV_EVENT_EOF) {
    printf("evnt eof\n");
    bufferevent_disable(bev, EV_READ | EV_WRITE);
    bufferevent_free(bev);
    event_base *tmp_base = bufferevent_get_base(bev);
    event_base_loopbreak(tmp_base);
    return;
  }
}

void HttpServer::read_cb(bufferevent *bev, void *ctx) {
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  size_t size = evbuffer_get_length(input);
  char *message = new char[size + 1];
  evbuffer_copyout(input, message, size);
  message[size] = '\0';
  printf("Server message is %s\n", message);
  delete[] message;

  evbuffer_add_buffer(output, input);
}

void HttpServer::accept_error_cb(struct evconnlistener *listener, void *ctx) {
  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr, "Got an error %d (%s) on the listener. Shutting down.\n", err,
          evutil_socket_error_to_string(err));
  event_base_loopexit(base, nullptr);
}
