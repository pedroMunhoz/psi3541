#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_timer.h"

#include "wifiDebug.h"

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

#define KP_DIR 2.0
#define KP_DIR_TOTAL 10.0
#define KD_DIR 0.4
#define KI_DIR 1.0

typedef int Pin;

typedef enum {
    MESSAGE_MQTT_START,
    MESSAGE_MQTT_STOP,
    MESSAGE_MQTT_STATUS,
    MESSAGE_CAR_MOVE,
    MESSAGE_CAR_SET_CONFIG,
    MESSAGE_CAR_GET_CONFIG,
    MESSAGE_CAR_GET_STATUS,
    MESSAGE_UNKNOWN
} messenger_message_type_t;

typedef enum {
    STATE_FRENTE,
    STATE_TRAS,
    STATE_ESQ,
    STATE_DIR,
    STATE_ROT_ESQ,
    STATE_ROT_DIR,
    STATE_STOP
} CarState;
typedef struct {
    CarState state;
    int ref;
} Action;

typedef struct {
    float Kp;
    float Kp_total;
    float Kd;
    float Ki;
} CarConfig;

typedef struct {
    CarState state;
    int ref;
    int cur;
    bool done;
} CarStatus;

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