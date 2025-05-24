#ifndef MQTT_H
#define MQTT_H

#include <cJSON.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "mqtt_client.h"

#include "project.h"
#include "Messenger.h"

#define MAX_MQTT_TOPICS 10

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_AmazonRootCA1_pem_end");

typedef struct {
    const char *topic;
    void (*handler)(const char *data, int data_len);
} mqtt_topic_t;

void mqtt_app_start(void);
void task_publish_dht11(void *arg);

#endif // MQTT_H