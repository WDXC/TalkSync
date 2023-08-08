#include <Client.h>
#include <arpa/inet.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <string.h>
#include <sys/time.h>
#include <event2/http.h>

std::string Client::msg;
struct event_base *Client::base_ = nullptr;
bufferevent *Client::bev_ = nullptr;
bool Client::flag_ = false;
std::condition_variable Client::clientStarted_;
std::mutex Client::bevMutex_;
event *Client::watchdog_;

Client::Client() {}

Client::~Client() {}


bool Client::Connect(const std::string& dst_adr) {
  struct evhttp_uri *http_uri = NULL;
  if (dst_adr.empty()) {
      return false;
  }

  http_uri = evhttp_uri_parse(dst_adr.c_str());
  if (http_uri == NULL) {
      perror("malformed url\n");
      return false;
  }

  const char* scheme = evhttp_uri_get_scheme(http_uri);
  if (scheme == NULL || (strcasecmp(scheme, "https") != 0 &&
              strcasecmp(scheme, "http") != 0)) {
      perror("url must be http or https\n");
      return false;
  }

  const char* host = evhttp_uri_get_host(http_uri);
  if (host == NULL) {
      perror("url must have a host");
      return false;
  }
  int port = evhttp_uri_get_port(http_uri);
  if (port == -1) {
      port == (strcasecmp(scheme,"http") == 0) ? 80 : 443;
  }

  const char* path = evhttp_uri_get_path(http_uri);
  if (strlen(path) == 0) {
      path = "/";
  }

  char uri[256];
  const char* query = evhttp_uri_get_query(http_uri);
  if (query == NULL) {
      snprintf(uri, sizeof(uri) - 1, "%s", path);
  } else {
      snprintf(uri, sizeof(uri) -1, "%s?%s", path, query);
  }
  uri[sizeof(uri)-1] = '\0';

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
    sin.sin_addr.s_addr = htonl(0);

    timeval ct{0, 100};
    watchdog_ = event_new(base_, bufferevent_getfd(bev_), EV_PERSIST,
                          watchdog_cb, nullptr);
    event_add(watchdog_, &ct);

    bufferevent_setcb(bev_, read_cb, nullptr, event_cb, nullptr);
    if (bufferevent_enable(bev_, EV_READ)) {
      perror("bufferevent_enable()");
      return false;
    }
    struct evhttp_connection *evcon =
        evhttp_connection_base_bufferevent_new(base_, NULL, bev_, host, port);
    if (evcon) {
      perror("bufferevent_socket_connect() connect failed");
      bufferevent_free(bev_);
      bev_ = nullptr;
      return false;
    }
  }
  event_base_dispatch(base_);

  return true;
}

void Client::Disconnect() { flag_ = true; }

std::string Client::SendData(const std::string &data) {
    if (bev_ == nullptr) {
        return "";
    }
  const char *message = "Hello";
  bufferevent_write(bev_, message, strlen(message));
  return nullptr;
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

void Client::http_request_done(struct evhttp_request* req, void* ctx) {
    char buffer[256];
    int nread;

    if (!req || !evhttp_request_get_response_code(req)) {
        fprintf(stderr, "some request failed - no idea which one though!\n");
    }

    fprintf(stderr, "Response line: %d %s\n",
            evhttp_request_get_response_code(req),
            evhttp_request_get_response_code_line(req));

    while ( (nread = evbuffer_remove(evhttp_request_get_input_buffer(req),
                    buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, nread, 1, stdout);
    }
}
