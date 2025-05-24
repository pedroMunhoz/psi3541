#ifndef PROJECT_H
#define PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include <cJSON.h> 

#include "driver/gpio.h"
#include "dht.h"

#define LED_PIN 2
#define DHT_PIN 4

#define KEY_INT     'i'
#define KEY_FLOAT   'f'
#define KEY_STRING  's'
typedef struct {
    float temperature;
    float humidity;
} dht_data_t;

typedef enum {
    MESSAGE_LED,
    MESSAGE_DHT,
    MESSAGE_BLINK,
    MESSAGE_UNKNOWN
} messenger_message_type_t;

extern dht_sensor_type_t sensor_type;
extern float lastTemperature;
extern float lastHumidity;

dht_data_t dht_read();
char *create_json(const char *key, ...);

#endif // PROJECT_H