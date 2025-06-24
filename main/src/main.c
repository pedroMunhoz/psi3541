#include "project.h"

#include "nvs_flash.h"

#include "connect_wifi.h"
#include "Messenger.h"
#include "filesystem.h"
#include "server.h"
#include "mqtt.h"
#include "car.h"

#define TIME_TO_TEST 2000

static const char *TAG = "main";

typedef struct {
    Messenger messenger;
    Filesystem fs;
    myServer server;
    Mqtt mqtt;
    Car car;
} System;

static System sys;

void testCar() {
    Action action = {.state=STATE_FRENTE, .ref=100};
    int received;
    
    wifi_debug_printf("Frente: \n");     
    messenger_send_with_response_generic(&sys.messenger, MESSAGE_CAR_MOVE, (void*)&action, &received, sizeof(int));
    while(!sys.car.done)
        vTaskDelay(pdMS_TO_TICKS(500));

    wifi_debug_printf("Tras: \n");     
    action.state = STATE_TRAS;
    messenger_send_with_response_generic(&sys.messenger, MESSAGE_CAR_MOVE, (void*)&action, &received, sizeof(int));
    while(!sys.car.done)
        vTaskDelay(pdMS_TO_TICKS(500));
    
    wifi_debug_printf("Esq: \n");
    action.state=STATE_ROT_DIR;
    action.ref=90;
    messenger_send_with_response_generic(&sys.messenger, MESSAGE_CAR_MOVE, (void*)&action, &received, sizeof(int));
    while(!sys.car.done)
        vTaskDelay(pdMS_TO_TICKS(500));
    
    wifi_debug_printf("Dir: \n");
    action.state=STATE_ROT_ESQ;
    action.ref=180;
        messenger_send_with_response_generic(&sys.messenger, MESSAGE_CAR_MOVE, (void*)&action, &received, sizeof(int));
    while(!sys.car.done)
        vTaskDelay(pdMS_TO_TICKS(500));

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

    filesystem_start(&sys.fs);
    connect_wifi();

    while (!wifi_connect_status) {}

    server_init(&sys.server);
    server_setMessenger(&sys.server, &sys.messenger);

    mqtt_init(&sys.mqtt);
    mqtt_setMessenger(&sys.mqtt, &sys.messenger);
    
    wifi_debug_init();

    messenger_message_t m = {.type=MESSAGE_MQTT_START, .int_data=0};
    messenger_send_message(&sys.messenger, &m);
    // testCar();
}