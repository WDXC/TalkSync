#include <Client.h>
#include <arpa/inet.h>
#include <event2/thread.h>
#include <string.h>
#include <sys/time.h>

std::string Client::msg;
struct event_base *Client::base_ = nullptr;
bufferevent *Client::bev_ = nullptr;
bool Client::flag_ = false;
std::condition_variable Client::clientStarted_;
std::mutex Client::bevMutex_;
event *Client::watchdog_;

Client::Client() {}

Client::~Client() {}

bool Client::Connect(const std::string &serverAddress, int port) {
  evthread_use_pthreads();
  base_ = event_base_new();
  if (!base_) {
    perror("event_base_new()");
    return false;
  }

  {
    std::lock_guard<std::mutex> lock(bevMutex_);
    bev_ = bufferevent_socket_new(
        base_, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    if (!bev_) {
      perror("bufferevnet_socket_new()");
      return false;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(serverAddress.c_str());

    timeval ct{0, 100};
    watchdog_ = event_new(base_, bufferevent_getfd(bev_), EV_PERSIST,
                          watchdog_cb, nullptr);
    event_add(watchdog_, &ct);

    bufferevent_setcb(bev_, read_cb, nullptr, event_cb, nullptr);
    if (bufferevent_enable(bev_, EV_READ)) {
      perror("bufferevent_enable()");
      return false;
    }
    int res =
        bufferevent_socket_connect(bev_, (struct sockaddr *)&sin, sizeof(sin));
    if (res) {
      perror("bufferevent_socket_connect()");
      return false;
    }
  }
  clientStarted_.notify_all();
  event_base_dispatch(base_);

  return true;
}

void Client::Disconnect() { flag_ = true; }

bool Client::SendData(const std::string &data) {
  const char *message = "Hello";
  std::unique_lock<std::mutex> lock(bevMutex_);
  clientStarted_.wait(lock, []() { return bev_ != nullptr; });
  bufferevent_write(bev_, message, strlen(message));
  return true;
}

const char *Client::GetMsg() { return msg.c_str(); }

void Client::event_cb(bufferevent *bev, short events, void *arg) {
  if (events & BEV_EVENT_CONNECTED) {
    printf("Client connected\n");
    return;
  } else if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    if (events & BEV_EVENT_ERROR) {
      printf("Error\n");
    }
    printf("Connected Close\n");
    bufferevent_free(bev);
    exit(1);
  }
}

void Client::read_cb(bufferevent *bev, void *ctx) {
  struct evbuffer *in = bufferevent_get_input(bev);
  size_t size = evbuffer_get_length(in);
  char *message = new char[size + 1];
  evbuffer_copyout(in, message, size);
  message[size] = '\0';
  printf("The return message is %s\n", message);
  Client::msg = message;
  evbuffer_drain(in, size);
  delete[] message;
}

void Client::watchdog_cb(int fd, short events, void *arg) {
  if (flag_) {
    if (base_) {
      bufferevent_disable(bev_, EV_READ);
      bufferevent_free(bev_);
      event_base_loopbreak(base_);
      event_base_free(base_);
    }
  }
  flag_ = false;
}
