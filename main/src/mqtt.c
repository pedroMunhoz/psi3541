#include "mqtt.h"
#include "sdkconfig.h"

static const char *TAG = "mqtts_example";

#define MAX_MQTT_REQ_LEN 128

static TaskHandle_t dht_task_handle = NULL;



static char *create_json(const char *key, ...) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    va_list args;
    va_start(args, key);

    const char *current_key = key;
    while (current_key != NULL) {
        json_key_type_t type = (json_key_type_t)va_arg(args, int);

        if (type == JSON_TYPE_INT) {
            int value = va_arg(args, int);
            cJSON_AddNumberToObject(json, current_key, value);
        } else if (type == JSON_TYPE_FLOAT) {
            double value = va_arg(args, double);
            cJSON_AddNumberToObject(json, current_key, value);
        } else if (type == JSON_TYPE_STRING) {
            const char *value = va_arg(args, const char *);
            cJSON_AddStringToObject(json, current_key, value);
        } else {
            cJSON_Delete(json);
            va_end(args);
            return NULL;
        }

        current_key = va_arg(args, const char *);
    }

    va_end(args);

    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    return response;
}

static bool mqtt_extract_data_from_json(const char *json_str, const char *key, json_key_type_t type, void *out_val) {
    cJSON *json = cJSON_Parse(json_str);
    if (!json) return false;

    cJSON *item = cJSON_GetObjectItem(json, key);
    bool valid = false;

    switch (type) {
        case JSON_TYPE_INT:
            if (cJSON_IsNumber(item)) {
                *(int *)out_val = item->valueint;
                valid = true;
            }
            break;
        case JSON_TYPE_FLOAT:
            if (cJSON_IsNumber(item)) {
                *(float *)out_val = (float)item->valuedouble;
                valid = true;
            }
            break;
        case JSON_TYPE_DOUBLE:
            if (cJSON_IsNumber(item)) {
                *(double *)out_val = item->valuedouble;
                valid = true;
            }
            break;
        case JSON_TYPE_STRING:
            if (cJSON_IsString(item) && item->valuestring) {
                strncpy((char *)out_val, item->valuestring, MQTT_MAX_REQ_LEN - 1);
                ((char *)out_val)[MQTT_MAX_REQ_LEN - 1] = '\0';
                valid = true;
            }
            break;
        case JSON_TYPE_BOOL:
            if (cJSON_IsBool(item)) {
                *(bool *)out_val = cJSON_IsTrue(item);
                valid = true;
            }
            break;
    }

    cJSON_Delete(json);
    return valid;
}

/*###############################################################*/


// static esp_err_t mqtt_dht11_handler(Mqtt *mqtt, const char *payload, int payload_len) {
//     dht_data_t dht_data;
//     bool ok = messenger_send_with_response_generic(
//         mqtt->messenger,
//         MESSAGE_DHT,
//         NULL,
//         &dht_data,
//         sizeof(dht_data_t)
//     );

//     if (ok) {
//         char *json = create_json("temperature", JSON_TYPE_FLOAT, dht_data.temperature,
//                                  "humidity", JSON_TYPE_FLOAT, dht_data.humidity, NULL);
//         esp_mqtt_client_publish(mqtt->client, "/topic/dht11/response", json, 0, 0, 0);
//         free(json);
//         return ESP_OK;
//     }
//     return ESP_FAIL;
// }

/*###############################################################*/

static mqtt_api_route_t mqtt_api_routes[] = {
    // { "/topic/led",    mqtt_led_handler },
    // { "/topic/blink",  mqtt_blink_handler },
    // { "/topic/dht11",  mqtt_dht11_handler }
};

/*###############################################################*/

#define MQTT_API_ROUTE_COUNT (sizeof(mqtt_api_routes)/sizeof(mqtt_api_routes[0]))

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    Mqtt *mqtt = (Mqtt *)handler_args;
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        mqtt->connected = true;
        for (int i = 0; i < MQTT_API_ROUTE_COUNT; i++) {
            esp_mqtt_client_subscribe(mqtt->client, mqtt_api_routes[i].topic, 0);
        }
        break;
    case MQTT_EVENT_DATA:
        for (int i = 0; i < MQTT_API_ROUTE_COUNT; i++) {
            if (strncmp(event->topic, mqtt_api_routes[i].topic, event->topic_len) == 0 &&
                strlen(mqtt_api_routes[i].topic) == event->topic_len) {
                // Null-terminate payload for JSON parsing
                char payload[MAX_MQTT_REQ_LEN];
                int len = event->data_len < MAX_MQTT_REQ_LEN-1 ? event->data_len : MAX_MQTT_REQ_LEN-1;
                memcpy(payload, event->data, len);
                payload[len] = '\0';
                mqtt_api_routes[i].handler(mqtt, payload, len);
                break;
            }
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        mqtt->connected = false;
        if (dht_task_handle != NULL) {
            vTaskDelete(dht_task_handle);
            dht_task_handle = NULL;
        }
        break;
    default:
        break;
    }
}

/*###############################################################*/

// Example publish task for DHT11
static void task_publish_dht11(void *arg) {
    // Mqtt *mqtt = (Mqtt *)arg;
    // dht_data_t dht_data;
    // while (1) {
    //     bool ok = messenger_send_with_response_generic(
    //         mqtt->messenger, MESSAGE_DHT, NULL, &dht_data, sizeof(dht_data_t));
    //     if (ok) {
    //         char *json_str = create_json("temperature", JSON_TYPE_FLOAT, dht_data.temperature,
    //                                      "humidity", JSON_TYPE_FLOAT, dht_data.humidity, NULL);
    //         esp_mqtt_client_publish(mqtt->client, "/topic/dht11", json_str, 0, 0, 0);
    //         if (json_str) free(json_str);
    //     }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
}

// Add more publish tasks as needed, e.g. task_publish_led, etc.

/*###############################################################*/

static void handle_mqtt_message(messenger_message_t *message, void *context) {
    Mqtt *mqtt = (Mqtt *)context;

    switch (message->type) {
        case MESSAGE_MQTT_START: {
            int idx = message->int_data;
            int result = 0;
            if (idx >= 0 && idx < mqtt->pub_task_count && !mqtt->pub_tasks[idx].active) {
                result = xTaskCreate(mqtt->pub_tasks[idx].fn, mqtt->pub_tasks[idx].name, 4096, mqtt, 5, &mqtt->pub_tasks[idx].handle);
                if (result) {
                    mqtt->pub_tasks[idx].active = true;
                } else {
                    mqtt->pub_tasks[idx].active = false;
                }
            }
            messenger_setResponse(message, (void*)&result);
            break;
        }
        case MESSAGE_MQTT_STOP: {
            int idx = message->int_data;
            int result = 1;
            if (idx >= 0 && idx < mqtt->pub_task_count && mqtt->pub_tasks[idx].active) {
                if (mqtt->pub_tasks[idx].handle != NULL) {
                    vTaskDelete(mqtt->pub_tasks[idx].handle);
                    mqtt->pub_tasks[idx].handle = NULL;
                    mqtt->pub_tasks[idx].active = false;
                }
            }
            messenger_setResponse(message, (void*) &result);
            break;
        }
        case MESSAGE_MQTT_STATUS: {
                static bool status[MQTT_MAX_PUB_TASKS];
                for (int i = 0; i < mqtt->pub_task_count; ++i)
                    status[i] = mqtt->pub_tasks[i].active;
                messenger_setResponse(message, (void*) status);
            }
            break;
        default:
            break;
    }
}

void mqtt_setMessenger(Mqtt *mqtt, Messenger *messenger) {
    mqtt->messenger = messenger;
    messenger_register_handler(mqtt->messenger, MESSAGE_MQTT_START, handle_mqtt_message, mqtt);
    messenger_register_handler(mqtt->messenger, MESSAGE_MQTT_STOP, handle_mqtt_message, mqtt);
    messenger_register_handler(mqtt->messenger, MESSAGE_MQTT_STATUS, handle_mqtt_message, mqtt);
}

void mqtt_init(Mqtt *mqtt) {
    ESP_LOGI(TAG, "Initializing MQTT module...");
    mqtt->messenger = NULL;
    mqtt->connected = false;

    char mqtt_uri[256];
    snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtts://%s:%d", CONFIG_BROKER_URI, CONFIG_BROKER_PORT);

    ESP_LOGI(TAG, "MQTT URI: %s", mqtt_uri);

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = mqtt_uri,
        .broker.verification.certificate = (const char *)server_cert_pem_start,
        .credentials = {
            .authentication = {
                .certificate = (const char *)client_cert_pem_start,
                .key = (const char *)client_key_pem_start,
            },
        }
    };

    mqtt->client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt->client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt);
    esp_mqtt_client_start(mqtt->client);

    mqtt->pub_task_count = 1;
    mqtt->pub_tasks[0] = (mqtt_publish_task_entry_t){
        .name = "task_publish_dht11",
        .fn = task_publish_dht11,
        .handle = NULL,
        .active = false
    };
    ESP_LOGI(TAG, "MQTT module initialized.");
}