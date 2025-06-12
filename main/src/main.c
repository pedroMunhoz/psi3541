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
    messenger_message_t message = {
        .type = MESSAGE_CAR_MOVE
    };
    Action acao;

    for (int i=40; i<=100; i+= 10) {
        wifi_debug_printf("########################################\n\tTESTANDO PARA POT: %d%%\n########################################\n", i);
        
        wifi_debug_printf("Frente: \n");
        acao.move = FRENTE;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));


        wifi_debug_printf("Tras: \n");
        acao.move = TRAS;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));
        wifi_debug_printf("OK\n");


        wifi_debug_printf("Rotacao esquerda: \n");
        acao.move = ROT_ESQUERDA;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));
        wifi_debug_printf("OK\n");


        wifi_debug_printf("Rotacao direita: \n");
        acao.move = ROT_DIREITA;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));
        wifi_debug_printf("OK\n");


        wifi_debug_printf("Esquerda: \n");
        acao.move = ESQUERDA;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));
        wifi_debug_printf("OK\n");


        wifi_debug_printf("Direita: \n");
        acao.move = DIREITA;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));
        wifi_debug_printf("OK\n");


        wifi_debug_printf("Parar: \n");
        acao.move = PARAR;
        acao.pot = i;
        
        message.data = (void*)&acao;
        messenger_send_message(&sys.messenger, &message);
        
        vTaskDelay(pdMS_TO_TICKS(TIME_TO_TEST));
        wifi_debug_printf("OK\n");
    }

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