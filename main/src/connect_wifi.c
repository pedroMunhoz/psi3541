#include "connect_wifi.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_netif.h"

#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

#include "mdns.h"

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
int wifi_connect_status = 0;

static const char *TAG = "wifi_connect";

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "WiFi event: base=%s, id=%ld", event_base, event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to the AP...");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        wifi_connect_status = 0;
        ESP_LOGI(TAG, "Connection to the AP failed");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP received");
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_connect_status = 1;
    }
}

void ask_for_credentials(char *ssid, char *password)
{
    printf("Enter WiFi SSID: ");
    fgets(ssid, 32, stdin);
    ssid[strcspn(ssid, "\n")] = '\0'; // remove newline

    printf("Enter WiFi Password: ");
    fgets(password, 64, stdin);
    password[strcspn(password, "\n")] = '\0'; // remove newline
}

void start_mdns_service()
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("munhoz"));
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP32 Munhoz Host"));
    ESP_LOGI(TAG, "mDNS started: http://munhoz.local");
}

void connect_wifi(void)
{
    ESP_LOGI(TAG, "Initializing WiFi...");
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    char ssid[32] = ESP_WIFI_SSID;
    char password[64] = ESP_WIFI_PASSWORD;

    while (1)
    {
        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_LOGI(TAG, "Configuring WiFi: SSID=%s", ssid);
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "Connected to SSID:%s", ssid);
            break;
        }
        else if (bits & WIFI_FAIL_BIT)
        {
            ESP_LOGW(TAG, "Failed to connect to SSID:%s", ssid);
            s_retry_num = 0;
            ESP_ERROR_CHECK(esp_wifi_stop());
            ask_for_credentials(ssid, password);
            xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vEventGroupDelete(s_wifi_event_group);

    // Start mDNS
    start_mdns_service();
    ESP_LOGI(TAG, "WiFi initialization complete.");
}
