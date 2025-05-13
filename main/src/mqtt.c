#include "mqtt.h"
#include "sdkconfig.h"

static const char *TAG = "mqtts_example";

esp_mqtt_client_handle_t client_mqtt;

static mqtt_topic_t mqtt_topic_list[MAX_MQTT_TOPICS];
static int mqtt_topic_count = 0;

void mqtt_register_topic_handler(const char *topic, void (*handler)(const char *data, int data_len)) {
    if (mqtt_topic_count < MAX_MQTT_TOPICS) {
        mqtt_topic_list[mqtt_topic_count].topic = topic;
        mqtt_topic_list[mqtt_topic_count].handler = handler;
        mqtt_topic_count++;
        ESP_LOGI(TAG, "Registered handler for topic: %s", topic);
    } else {
        ESP_LOGE(TAG, "Maximum number of MQTT topics reached");
    }
}

void handle_led(const char* data, int data_len) {

    int lvalue=-1;

    cJSON *root = cJSON_Parse(data);
    cJSON *led_value = cJSON_GetObjectItem(root, "led");
    if (led_value && led_value->type == cJSON_Number) {
        lvalue = led_value->valueint;
    }

    cJSON_Delete(root);

    post_office_message_t message = {
        .type = MESSAGE_LED,
        .data = (lvalue != -1) ? (int *)&lvalue : NULL,
        .response_queue = NULL
    };

    post_office_send_message(&message);
}

void handle_blink(const char* data, int data_len) {
    int bvalue = -1;
    int bfreq = -1;

    cJSON *root = cJSON_Parse(data);
    cJSON *blink_value = cJSON_GetObjectItem(root, "blink");
    if (blink_value && blink_value->type == cJSON_Number) {
        bvalue = blink_value->valueint;
    }
    cJSON *blink_freq = cJSON_GetObjectItem(root, "frequency");
    if (blink_freq && blink_freq->type == cJSON_Number) {
        bfreq = blink_freq->valueint;
    }

    cJSON_Delete(root);

    post_office_message_t message = {
        .type = MESSAGE_BLINK,
        .data = (bfreq != -1) ? (int *)&bfreq : NULL,
        .response_queue = NULL
    };

    post_office_send_message(&message);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        for (int i = 0; i < mqtt_topic_count; i++) {
            esp_mqtt_client_subscribe(client_mqtt, mqtt_topic_list[i].topic, 0);
            ESP_LOGI(TAG, "Subscribed to topic: %s", mqtt_topic_list[i].topic);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);

        for (int i = 0; i < mqtt_topic_count; i++) {
            if (strncmp(event->topic, mqtt_topic_list[i].topic, event->topic_len) == 0) {
                mqtt_topic_list[i].handler(event->data, event->data_len);
                break;
            }
        }

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    char mqtt_uri[256];
    snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtts://%s:%d", CONFIG_BROKER_URI, CONFIG_BROKER_PORT);

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

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    client_mqtt = client;
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "after mqtt client start");

    mqtt_register_topic_handler("/topic/led", handle_led);
    mqtt_register_topic_handler("/topic/blink", handle_blink);
}

void task_publish_dht11(void *arg) {
    dht_data_t dht_data;

    while(1) {
        dht_data = dht_read();

        char *json_string = create_json("temperature", KEY_FLOAT, dht_data.temperature, "humidity", KEY_FLOAT, dht_data.humidity, NULL);

        esp_mqtt_client_publish(client_mqtt, "/topic/dht11", json_string, 0, 0, 0);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        if (json_string) {
            cJSON_free((void*)json_string); // Free the memory allocated by cJSON_Print
        }

    }
}
