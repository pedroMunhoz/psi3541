#include "wifiDebug.h"

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "wifi_debug";
static int client_sock = -1;

static void wifi_debug_task(void *pvParameters) {
    char addr_str[128];
    int listen_sock;
    struct sockaddr_in dest_addr, source_addr;
    socklen_t addr_len = sizeof(source_addr);

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(WIFI_DEBUG_PORT);

    if (bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    listen(listen_sock, 1);
    ESP_LOGI(TAG, "TCP debug server listening on port %d", WIFI_DEBUG_PORT);

    while (1) {
        ESP_LOGI(TAG, "Waiting for client connection...");
        client_sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            continue;
        }

        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "Client connected: %s", addr_str);

        // Wait until client disconnects
        while (1) {
            char tmp[1];
            int ret = recv(client_sock, tmp, 1, MSG_PEEK | MSG_DONTWAIT);
            if (ret == 0) {
                ESP_LOGI(TAG, "Client disconnected");
                close(client_sock);
                client_sock = -1;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

void wifi_debug_init(void) {
    xTaskCreate(wifi_debug_task, "wifi_debug", 4096, NULL, 5, NULL);
}

void wifi_debug_printf(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("%s", buffer);  // Still print to console

    if (client_sock > 0) {
        send(client_sock, buffer, len, 0);
    }
}
