#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_server.h"

#include "cJSON.h"
#include "project.h"
#include "Messenger.h"
#include "filesystem.h"

#define MAX_STRING_LEN 128
#define MAX_REQ_LEN 128

#define URI_ENTRY(_uri, _method, _handler) \
    { .uri = _uri, .method = _method, .handler = _handler, .user_ctx = NULL }

typedef struct {
    const char *uri;
    const char *filepath;
    const char *mime;
} static_route_t;

typedef struct {
    const char* uri;
    httpd_method_t method;
    esp_err_t (*handler)(httpd_req_t *req);
} api_route_t;

typedef struct {
    httpd_config_t config;
    Messenger* messenger;

    httpd_handle_t handle;
} myServer;

void server_init(myServer* server);
void server_setMessenger(myServer* server, Messenger* messenger);

#endif // SERVER_H