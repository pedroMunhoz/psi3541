#include "my_dht.h"

static const char *TAG = "MyDHT"; // TAG for debug

dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;

float lastTemperature = 25.0; // Valor inicial padrão
float lastHumidity = 70.0;

bool is_dht_present() {
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
    int level = gpio_get_level(DHT_PIN);
    return level == 0; // Espera que o sensor puxe para LOW
}

dht_data_t dht_read() {
    dht_data_t data = {0};

    if (!is_dht_present()) {
        ESP_LOGE(TAG, "DHT sensor not detected on pin %d. Simulating values based on last.", DHT_PIN);
        data.temperature = lastTemperature + (rand() % 100) / 100.0 - 0.5;
        data.humidity = lastHumidity + (rand() % 100) / 100.0 - 0.5;
        lastTemperature = data.temperature;
        lastHumidity = data.humidity;
        return data;
    }

    esp_err_t result;
    for (int i = 0; i < DHT_MAX_RETRIES; i++) {
        result = dht_read_float_data(sensor_type, DHT_PIN, &data.humidity, &data.temperature);
        if (result == ESP_OK &&
            data.temperature > DHT_TEMP_MIN && data.temperature < DHT_TEMP_MAX &&
            data.humidity > DHT_HUM_MIN && data.humidity <= DHT_HUM_MAX) {

            lastTemperature = data.temperature;
            lastHumidity = data.humidity;
            return data;
        }

        ESP_LOGW(TAG, "DHT read failed (attempt %d): %s", i + 1, esp_err_to_name(result));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGE(TAG, "Returning last known values after retries.");
    data.temperature = lastTemperature;
    data.humidity = lastHumidity;
    return data;
}

/* ###################### MODULE TASKS ######################*/

/* ###################### MODULE HANDLERS ######################*/
static void handle_dht(messenger_message_t *message) {
    dht_data_t data = dht_read();
    ESP_LOGI("DHT", "Temperature: %.2f, Humidity: %.2f", data.temperature, data.humidity);
    
    messenger_setResponse(message, &data);
}

/* ###################### MODULE EXTERNAL FUNCTIONS ######################*/

void init_dht() {
    gpio_reset_pin(DHT_PIN);
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT_OUTPUT);

    messenger_register_handler(MESSAGE_DHT, handle_dht);
}
