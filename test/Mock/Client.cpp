#include <Client.h>
#include <arpa/inet.h>
#include <string.h>

std::string Client::msg = "";

Client::Client() {}

Client::~Client() {}

bool Client::Connect(const std::string &serverAddress, int port) {
  base_ = event_base_new();
  if (!base_) {
    perror("event_base_new()");
    return false;
  }

  struct bufferevent *bev;
  bev = bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    perror("bufferevnet_socket_new()");
    return false;
  }

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(serverAddress.c_str());

  bufferevent_setcb(bev, read_cb, nullptr, event_cb, nullptr);
  if (bufferevent_enable(bev, EV_READ)) {
    perror("bufferevent_enable()");
    return false;
  }
  int res =
      bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin));
  if (res) {
    perror("bufferevent_socket_connect()");
    return false;
  }

  bev_ = std::shared_ptr<bufferevent>(
      bev, [](bufferevent *bev) { bufferevent_free(bev); });

  event_base_dispatch(base_);

  return true;
}

void Client::Disconnect() {
  event_base_loopexit(base_, nullptr);
  event_base_free(base_);
  bev_.reset();
  return;
}

bool Client::SendData(const std::string &data) {
  struct evbuffer *output = bufferevent_get_output(bev_.get());
  evbuffer_add(output, data.c_str(), data.size());
  evbuffer_write(output, bufferevent_getfd(bev_.get()));
  return true;
}

std::string Client::GetMsg() { return msg; }

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

void read_cb(bufferevent *bev, void *ctx) {
  struct evbuffer *in = bufferevent_get_input(bev);
  size_t size = evbuffer_get_length(in);
  char *message = new char[size + 1];
  evbuffer_copyout(in, message, size);
  message[size] = '\0';
  printf("The return message is %s", message);
  Client::msg = message;
  evbuffer_drain(in, size);
  delete[] message;
}
