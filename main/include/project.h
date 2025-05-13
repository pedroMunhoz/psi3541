#ifndef PROJECT_H
#define PROJECT_H

#define LED_PIN 2
#define DHT_PIN 4

typedef struct {
    float temperature;
    float humidity;
} dht_data_t;

typedef enum {
    MESSAGE_LED,
    MESSAGE_DHT,
    MESSAGE_UNKNOWN
} post_office_message_type_t;

#endif // PROJECT_H