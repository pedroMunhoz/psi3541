#include "nvs_flash.h"

#include "project.h"

#include "connect_wifi.h"
#include "Messenger.h"
#include "server.h"
#include "mqtt.h"
#include "my_led.h"
#include "my_dht.h"

static const char *TAG = "espressif"; // TAG for debug

void app_main() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Messenger
    messenger_init();
    
    // Initilize project modules
    init_dht();
    init_led();

    connect_wifi();

    if (wifi_connect_status) {
        ESP_LOGI(TAG, "SPIFFS Web Server is running ... ...\n");
        setup_server();

        mqtt_app_start();
        xTaskCreate(task_publish_dht11, "task_publish_dht11", 4096, 0,5,0);
    }
}