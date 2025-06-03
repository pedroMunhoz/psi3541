#include "project.h"

#include "nvs_flash.h"

#include "connect_wifi.h"
#include "Messenger.h"
#include "filesystem.h"
#include "server.h"
// #include "mqtt.h"
#include "car.h"

static const char *TAG = "main";

typedef struct {
    Messenger messenger;
    Filesystem fs;
    myServer server;
    // Mqtt mqtt;
    Car car;
} System;

static System sys;

void testCar() {
    motor_setDirection(&sys.car.motorR, MOTOR_HORARIO);
    vTaskDelay(pdMS_TO_TICKS(5000));

    motor_setDirection(&sys.car.motorR, MOTOR_ANTIHORARIO);
    vTaskDelay(pdMS_TO_TICKS(5000));

    motor_setDirection(&sys.car.motorR, MOTOR_PARADO);
    motor_setDirection(&sys.car.motorL, MOTOR_HORARIO);
    vTaskDelay(pdMS_TO_TICKS(5000));

    motor_setDirection(&sys.car.motorL, MOTOR_ANTIHORARIO);
    vTaskDelay(pdMS_TO_TICKS(5000));

    motor_setDirection(&sys.car.motorL, MOTOR_PARADO);
    car_move(&sys.car, FRENTE);
    vTaskDelay(pdMS_TO_TICKS(5000));

    car_move(&sys.car, TRAS);
    vTaskDelay(pdMS_TO_TICKS(5000));

    car_move(&sys.car, ROT_ESQUERDA);
    vTaskDelay(pdMS_TO_TICKS(5000));

    car_move(&sys.car, ROT_DIREITA);
    vTaskDelay(pdMS_TO_TICKS(5000));

    car_move(&sys.car, PARAR);
    vTaskDelay(pdMS_TO_TICKS(5000));

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
    car_init(&sys.car, CAR_IN_1, CAR_IN_2, CAR_IN_4, CAR_IN_3, 2, 4);
    car_setMessenger(&sys.car, &sys.messenger);

    filesystem_start(&sys.fs);
    connect_wifi();

    xTaskCreate(testCar, "test_car", 2048, NULL, 5, NULL);

    if (wifi_connect_status) {
        server_setMessenger(&sys.server, &sys.messenger);
        server_init(&sys.server);

        // mqtt_init(&sys.mqtt);
        // mqtt_setMessenger(&sys.mqtt, &sys.messenger);
        
        // int idx = 0;
        // messenger_message_t msg = {.type = MESSAGE_MQTT_START, .data = &idx, .response_queue = NULL};
        // messenger_send_message(&sys.messenger, &msg);
    }
}