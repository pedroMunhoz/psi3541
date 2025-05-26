#ifndef MQTT_H
#define MQTT_H

#include <stdlib.h>

#include <cJSON.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "project.h"
#include "Messenger.h"
#include <stdbool.h>

#define MAX_MQTT_TOPICS 10
#define MQTT_MAX_REQ_LEN 128

typedef struct Mqtt Mqtt;
typedef esp_err_t (*mqtt_api_handler_t)(Mqtt *mqtt, const char *payload, int payload_len);

typedef struct {
    const char *topic;
    mqtt_api_handler_t handler;
    void *ctx;
} mqtt_api_route_t;

typedef void (*mqtt_publish_task_fn)(void *arg);

typedef struct {
    const char *name;
    mqtt_publish_task_fn fn;
    TaskHandle_t handle;
    bool active;
} mqtt_publish_task_entry_t;

struct Mqtt {
    esp_mqtt_client_handle_t client;
    Messenger *messenger;
    bool connected;
    mqtt_publish_task_entry_t pub_tasks[MQTT_MAX_PUB_TASKS];
    int pub_task_count;
};

void mqtt_init(Mqtt *mqtt);
void mqtt_setMessenger(Mqtt *mqtt, Messenger *messenger);

#endif // MQTT_H