#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"

#define PIN_CAR_IN1 15
#define PIN_CAR_IN2 2
#define PIN_CAR_IN3 4
#define PIN_CAR_IN4 16
#define PIN_CAR_EN_A 17
#define PIN_CAR_EN_B 5

#define PIN_ENCODER_R 18
#define PIN_ENCODER_L 19

#define PIN_MPU_SCL 22
#define PIN_MPU_SDA 21
#define PIN_MPU_INT 23

#define MQTT_MAX_PUB_TASKS 4

#define NUM_ENCODERS 2

typedef int Pin;

typedef enum {
    MESSAGE_MQTT_START,
    MESSAGE_MQTT_STOP,
    MESSAGE_MQTT_STATUS,
    MESSAGE_CAR_MOVE,
    MESSAGE_UNKNOWN
} messenger_message_type_t;

typedef struct {
    float temperature;
    float humidity;
} dht_data_t;

#define LED_UPDATE_PERIOD 200   //ms
#define DHT_UPDATE_PERIOD 1000  //ms

typedef enum {
    JSON_TYPE_INT,
    JSON_TYPE_FLOAT,
    JSON_TYPE_STRING,
    JSON_TYPE_DOUBLE,
    JSON_TYPE_BOOL
} json_key_type_t;

// AWS keys
extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_AmazonRootCA1_pem_end");


#endif // PROJECT_H