#ifndef MYDHT_H
#define MYDHT_H

#define DHT_MAX_RETRIES 3
#define DHT_TEMP_MIN -40
#define DHT_TEMP_MAX 100
#define DHT_HUM_MIN 0
#define DHT_HUM_MAX 100

#include <dht.h>
#include "freertos/FreeRTOS.h"
#include "Messenger.h"

typedef struct {
    int pin;
    dht_sensor_type_t type;
    dht_data_t lastData;
    dht_data_t data;

    Messenger* messenger;
} myDHT;

void dht_init(myDHT* dht, int pin, dht_sensor_type_t type);
void dht_setMessenger(myDHT* dht, Messenger* messenger);
#endif