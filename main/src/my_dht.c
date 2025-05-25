#include "my_dht.h"

static const char *TAG = "MyDHT"; // TAG for debug

bool is_dht_present(myDHT* dht) {
    int level = gpio_get_level(dht->pin);
    return level == 0;
}

dht_data_t dht_read(myDHT* dht) {
    return dht->data;
}

/* ###################### MODULE TASKS ######################*/

void dht_task(void *pvParameters) {
    myDHT *dht = (myDHT *) pvParameters;

    while(1) {
        if (!is_dht_present(dht)) {
            //ESP_LOGE(TAG, "DHT sensor not detected on pin %d. Simulating values based on last.", dht->pin);
            
            // Mock new DHT data
            dht->data.temperature = dht->lastData.temperature + (rand() % 100) / 100.0 - 0.5;
            dht->data.humidity = dht->lastData.humidity + (rand() % 100) / 100.0 - 0.5;
            dht->lastData.temperature = dht->data.temperature;
            dht->lastData.humidity = dht->data.humidity;
        } else {
            dht_read_float_data(dht->type, dht->pin, &(dht->data.humidity), &(dht->data.temperature));
        }

        vTaskDelay(pdMS_TO_TICKS(DHT_UPDATE_PERIOD));
    }
}

/* ###################### MODULE HANDLERS ######################*/
static void handle_dht(messenger_message_t *message, void *context) {
    myDHT *dht = (myDHT *) context;
    dht_data_t data = dht_read(dht);
    messenger_setResponse(message, &data);
}

/* ###################### MODULE EXTERNAL FUNCTIONS ######################*/

void dht_init(myDHT* dht, int pin, dht_sensor_type_t type) {
    dht->pin = pin;
    dht->type = type;
    dht->lastData.temperature = 25.0;
    dht->lastData.humidity = 70.0;
    dht->data.temperature = 0.0;
    dht->data.humidity = 0.0;

    gpio_reset_pin(dht->pin);
    gpio_set_direction(dht->pin, GPIO_MODE_INPUT);

    xTaskCreate(dht_task, "dht_task", 2048, dht, 5, NULL);
}

void dht_setMessenger(myDHT* dht, Messenger* messenger) {
    dht->messenger = messenger;

    messenger_register_handler(dht->messenger, MESSAGE_DHT, handle_dht, dht);
}
