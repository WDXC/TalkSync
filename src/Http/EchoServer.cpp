#include "EchoServer.h"
#include <arpa/inet.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>

bufferevent *EchoServer::bev_;
event_base *EchoServer::base_;

void EchoServer::cb(int sock, short what, void *arg) {
  bufferevent_disable(bev_, EV_WRITE | EV_READ);
  bufferevent_free(bev_);

  event_base_loopbreak(base_);
  event_base_loopexit(base_, NULL);
  event_base_free(base_);

  bev_ = NULL;
  std::cout << "end base\n";
}

EchoServer::EchoServer() {}

EchoServer::~EchoServer() {
  listener_ = NULL;
  base_ = NULL;
  bev_ = NULL;
}

void EchoServer::RunLoop() {
  event_base_dispatch(base_);
  //  event_base_loop(base_, 0);
}

void EchoServer::Start() {
  struct sockaddr_in sin;

  int port = 9876;

  evthread_use_pthreads();
  base_ = event_base_new();
  if (!base_) {
    puts("Couldn't open event base");
    return;
  }

  /* Clear the sockaddr before using it, in case there are extra
   * platform-specific fields that can mess us up. */
  memset(&sin, 0, sizeof(sin));
  /* This is an INET address */
  sin.sin_family = AF_INET;
  /* Listen on 0.0.0.0 */
  sin.sin_addr.s_addr = htonl(0);
  /* Listen on the given port. */
  sin.sin_port = htons(port);

  listener_ = evconnlistener_new_bind(base_, accept_conn_cb, NULL,
                                      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                      -1, (struct sockaddr *)&sin, sizeof(sin));
  struct event *watchdog_event;
  struct timeval five_seconds = {2, 0};
  watchdog_event = event_new(base_, -1, EV_TIMEOUT | EV_READ | EV_PERSIST, cb,
                             (char *)"Reading event");

  event_add(watchdog_event, &five_seconds);
  if (!listener_) {
    perror("Couldn't create listener");
    return;
  }
  evconnlistener_set_error_cb(listener_, accept_error_cb);
}

void EchoServer::Stop() {
  evconnlistener_free(listener_);
  return;
}

void EchoServer::echo_read_cb(struct bufferevent *bev, void *ctx) {
  /* This callback is invoked when there is data to read on bev. */
  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  /* Copy all the data from the input buffer to the output buffer. */
  evbuffer_add_buffer(output, input);
}

void EchoServer::echo_event_cb(struct bufferevent *bev, short events,
                               void *ctx) {
  perror("Enter");
  if (events & BEV_EVENT_CONNECTED) {
    perror("Client connected.");
  }
  if (events & BEV_EVENT_ERROR)
    perror("Error from bufferevent");
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
  /* We got a new connection! Set up a bufferevent for it. */
  struct event_base *base = evconnlistener_get_base(listener);
  struct bufferevent *bev =
      bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

  // 手动触发 BEV_EVENT_CONNECTED 事件
  bufferevent_enable(bev, EV_READ | EV_WRITE);
  //  bufferevent_trigger(bev, EV_READ | EV_WRITE, BEV_EVENT_CONNECTED);

  // 其他设置和回调函数的操作
  bev_ = bev;
}

void EchoServer::accept_error_cb(struct evconnlistener *listener, void *ctx) {

  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr,
          "Got an error %d (%s) on the listener. "
          "Shutting down.\n",
          err, evutil_socket_error_to_string(err));

  event_base_loopexit(base, NULL);
}
