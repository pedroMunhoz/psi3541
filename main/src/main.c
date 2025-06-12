#include "project.h"

#include "nvs_flash.h"

#include "connect_wifi.h"
#include "Messenger.h"
// #include "filesystem.h"
// #include "server.h"
// #include "mqtt.h"
#include "car.h"

#define TIME_TO_TEST 2000

static const char *TAG = "main";

typedef struct {
    Messenger messenger;
    // Filesystem fs;
    // myServer server;
    // Mqtt mqtt;
    Car car;
} System;

static System sys;

void testCar() {
    Action action = {.state=STATE_FRENTE, .ref=100};
    messenger_message_t message = {
        .type = MESSAGE_CAR_MOVE
    };
    message.data = (void*)&action;
    wifi_debug_printf("Frente: \n");     
    messenger_send_message(&sys.messenger, &message);

    while(!sys.car.done)
        vTaskDelay(pdMS_TO_TICKS(100));

    action.state = STATE_TRAS;
    wifi_debug_printf("Frente: \n");     
    messenger_send_message(&sys.messenger, &message);

    vTaskDelete(NULL);
}

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
    car_init(&sys.car, PIN_CAR_IN1, PIN_CAR_IN2, PIN_CAR_IN3, PIN_CAR_IN4, PIN_CAR_EN_A, PIN_CAR_EN_B);
    car_setMessenger(&sys.car, &sys.messenger);
    encoder_init(&sys.car.motorL.encoder, PIN_ENCODER_L);
    encoder_init(&sys.car.motorR.encoder, PIN_ENCODER_R);

    // filesystem_start(&sys.fs);
    connect_wifi();

    if (wifi_connect_status) {
        wifi_debug_init();

        testCar();
    //     server_setMessenger(&sys.server, &sys.messenger);
    //     server_init(&sys.server);

        // mqtt_init(&sys.mqtt);
        // mqtt_setMessenger(&sys.mqtt, &sys.messenger);
        
        // int idx = 0;
        // messenger_message_t msg = {.type = MESSAGE_MQTT_START, .data = &idx, .response_queue = NULL};
        // messenger_send_message(&sys.messenger, &msg);

        wifi_debug_printf("Debug log: uptime = %lld ms\n", esp_timer_get_time() / 1000);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}