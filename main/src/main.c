#include "project.h"

#include "nvs_flash.h"

#include "connect_wifi.h"
#include "Messenger.h"
#include "filesystem.h"
#include "server.h"
#include "mqtt.h"
#include "my_led.h"
#include "my_dht.h"

static const char *TAG = "main";

typedef struct {
    Messenger messenger;
    myDHT dht;
    myLED led;
    Filesystem fs;
    myServer server;
    Mqtt mqtt;
} System;

static System sys;

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
        server_setMessenger(&sys.server, &sys.messenger);
        server_init(&sys.server);

        mqtt_init(&sys.mqtt);
        mqtt_setMessenger(&sys.mqtt, &sys.messenger);
        int idx = 0;
        messenger_message_t msg = {.type = MESSAGE_MQTT_START, .data = &idx, .response_queue = NULL};
        messenger_send_message(&sys.messenger, &msg);
    }
}