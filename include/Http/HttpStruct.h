#ifndef _HTTPSTRUCT_H_
#define _HTTPSTRUCT_H_

#include <cstddef>

#define MAX_URI_LENGTH 512

static const struct table_entry {
    const char* extension;
    const char* content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" },
	{ NULL, NULL },
};

struct options {
    int port;
    int iocp;
    int verbose;
    int max_body_size;

    int unlink;
    const char* unixsock;
    const char* bind;
    const char* docroot;
};

#endif
