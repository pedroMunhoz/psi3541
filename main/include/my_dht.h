#ifndef MYDHT_H
#define MYDHT_H

#define DHT_MAX_RETRIES 3
#define DHT_TEMP_MIN -40
#define DHT_TEMP_MAX 100
#define DHT_HUM_MIN 0
#define DHT_HUM_MAX 100

#include "Messenger.h"

void init_dht();

#endif