#include "project.h"

dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;

float lastTemperature = 25.0;
float lastHumidity = 70.0;

dht_data_t dht_read() {
    dht_data_t data;
    // esp_err_t result = dht_read_float_data(sensor_type, DHT_PIN, &data.humidity, &data.temperature);
    // if (result != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to read DHT data: %s", esp_err_to_name(result));
    //     data.temperature = 0;
    //     data.humidity = 0;
    // }

    // Mock data
    data.temperature = lastTemperature + (rand() % 100) / 100.0 - 0.5; // Random value close to 25ºC
    data.humidity = lastHumidity + (rand() % 100) / 100.0 - 0.5;    // Random value close to 70%
    lastTemperature = data.temperature;
    lastHumidity = data.humidity;

    return data;
}

char *create_json(const char *key, ...) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    va_list args;
    va_start(args, key);

    const char *current_key = key;
    while (current_key != NULL) {
        char type = (char)va_arg(args, int);

        if (type == 'i') {
            int value = va_arg(args, int);
            cJSON_AddNumberToObject(json, current_key, value);
        } else if (type == 'f') {
            double value = va_arg(args, double);
            cJSON_AddNumberToObject(json, current_key, value);
        } else if (type == 's') {
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