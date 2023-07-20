#ifndef _HTTPSERVER_H_
#define _HTTPSERVER_H_

#include "HttpStruct.h"
#include <event2/http.h>

class HttpServer {
    public:
        HttpServer();
        void Start();
    private:
        static void event_cb(bufferevent* bev, short events, void* arg);
        static void read_cb(bufferevent* bev, void* ctx);
    private:
        char* uri_root[MAX_URI_LENGTH];
};


#endif
