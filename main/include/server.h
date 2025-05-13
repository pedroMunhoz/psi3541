#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <esp_log.h>
#include <string.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include <sys/stat.h>
#include "esp_vfs.h"
#include <esp_http_server.h>
#include <cJSON.h>

#include "project.h"
#include "PostOffice.h"

// Function prototypes
httpd_handle_t setup_server(void);
esp_err_t static_file_handler(httpd_req_t *req);
esp_err_t led_handler(httpd_req_t *req);
esp_err_t dht11_handler(httpd_req_t *req);
esp_err_t blink_handler(httpd_req_t *req);

// Declare the list of URI handlers
extern httpd_uri_t uri_list[];

#endif // SERVER_H