#include "project.h"

#include "nvs_flash.h"

#include "connect_wifi.h"
#include "Messenger.h"
#include "filesystem.h"
#include "server.h"
// #include "mqtt.h"
#include "my_led.h"
#include "my_dht.h"

typedef struct {
    Messenger messenger;
    myDHT dht;
    myLED led;
    Filesystem fs;
    myServer server;
} System;

System sys;

void app_main() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Messenger
    messenger_init(&sys.messenger);
    
    // Initilize project modules
    dht_init(&sys.dht, DHT_PIN, DHT_TYPE_DHT11);
    dht_setMessenger(&sys.dht, &sys.messenger);

    led_init(&sys.led, LED_PIN);
    led_setMessenger(&sys.led, &sys.messenger);

    filesystem_start(&sys.fs);

    connect_wifi();

    if (wifi_connect_status) {
        server_init(&sys.server);
        server_setMessenger(&sys.server, &sys.messenger);

        // mqtt_app_start();
        // xTaskCreate(task_publish_dht11, "task_publish_dht11", 4096, 0,5,0);
    }
}