#include "HttpServer.h"
#include <stdlib.h>
#include <event2/keyvalq_struct.h>
#include <bits/types/struct_timeval.h>
#include <event2/thread.h>
#include <memory>
#include <cstring>

struct event_base *HttpServer::base_;

// customize deleter
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

void HttpServer::dump_request_cb(struct evhttp_request *req, void *arg) {
    const char* cmdtype;
    struct evkeyvalq* headers;
    struct evkeyval* header;
    struct evbuffer* buf;

    switch (evhttp_request_get_command(req)) {
        case EVHTTP_REQ_GET: cmdtype = "GET";
            break;
        case EVHTTP_REQ_POST: cmdtype = "POST";
            break;
        case EVHTTP_REQ_HEAD: cmdtype = "HEAD";
            break;
        case EVHTTP_REQ_PUT: cmdtype = "PUT";
            break;
        case EVHTTP_REQ_DELETE: cmdtype = "DELETE";
            break;
        case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS";
            break;
        case EVHTTP_REQ_TRACE: cmdtype = "TRACE";
            break;
        case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT";
            break;
        case EVHTTP_REQ_PATCH: cmdtype = "PATCH";
            break;
        default: cmdtype = "unknown";
            break;
    }

    printf("Received a %s request for %s\nHeaders:\n",
           cmdtype, evhttp_request_get_uri(req));

    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header;
        header = header->next.tqe_next) {
        printf(" %s: %s\n", header->key, header->value);
    }

    buf = evhttp_request_get_input_buffer(req);
    puts("Input data: <<<");
    while (evbuffer_get_length(buf)) {
        int n;
        char cbuf[128];
        n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
        if (n > 0)
            (void) fwrite(cbuf, 1, n, stdout);
    }
    puts(">>>");

    evhttp_send_reply(req, 200, "OK", NULL);
}

void HttpServer::send_document_cb(struct evhttp_request *req, void *arg) {
    struct evbuffer* evb = NULL;
    struct options* o = arg;
    const char* uri = evhttp_request_get_uri(req);
    struct evhttp_uri* decoded = NULL;
    const char* path;
    char* decoded_path;
    char* whole_path = NULL;
    size_t len;
    int fd = -1;
    struct stat st;

    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
        dump_request_cb(req, arg);
        return;
    }

    printf("Got a GET Request for <%s>\n", uri);

    /* Decode the URI*/
    decoded = evhttp_uri_parse(uri);
    if (!decoded) {
        printf("It is not a good URI. Sending BADREQUEST\n");
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    /* Let's see what path the user asked for */
    path = evhttp_uri_get_path(decoded);
    if (!path) path = "/";

    /* We need to decode it, to see what path the user really wanted. */
    decoded_path = evhttp_uridecode(path, 0, NULL);
    if (decoded_path == NULL)
        goto err;
    if (strstr(decoded_path, ".."))
        goto err;

    len = strlen(decoded_path) + strlen(o->docroot) + 2;
    if (!(whole_path = malloc(len))) {
        perror("malloc");
        goto err;
    }
err:
}
