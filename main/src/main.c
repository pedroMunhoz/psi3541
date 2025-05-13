#include "nvs_flash.h"

#include "project.h"

#include "connect_wifi.h"
#include "PostOffice.h"
#include "server.h"
#include "mqtt.h"

static const char *TAG = "espressif"; // TAG for debug

int blink_freq = 1;
void blink_task(void *pvParameter) {
    while (1) {
        int current_level = gpio_get_level(LED_PIN);
        gpio_set_level(LED_PIN, !current_level);
        vTaskDelay(pdMS_TO_TICKS(1000 / blink_freq));
    }
}

static void handle_led(post_office_message_t *message) {
    if (message->data != NULL) {
        int *state = (int *)message->data;
        gpio_set_level(LED_PIN, *state);
        ESP_LOGI("LED", "LED set to: %d", *state);
    } 
    int led_state = gpio_get_level(LED_PIN);
    ESP_LOGI("LED", "LED state: %d", led_state);

    setResponse(message, &led_state);
}

static void handle_dht(post_office_message_t *message) {
    dht_data_t data = dht_read();
    ESP_LOGI("DHT", "Temperature: %.2f, Humidity: %.2f", data.temperature, data.humidity);
    
    setResponse(message, &data);
}

static void handle_blink(post_office_message_t *message) {
    static int blink_state = 0;

    if (message->data != NULL) {
        int *frequency = (int *)message->data;
        blink_freq = *frequency;
        ESP_LOGI("BLINK", "Blink frequency set to: %d", blink_freq);
    } else {
        if (xTaskGetHandle("blink_task") != NULL) {
            vTaskDelete(xTaskGetHandle("blink_task"));
            ESP_LOGI("BLINK", "Blink task stopped");
            blink_state = 0;
            gpio_set_level(LED_PIN, 0);
        } else {
            xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);
            ESP_LOGI("BLINK", "Blink task started");
            blink_state = 1;
        }
    }
    setResponse(message, &blink_state);
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize GPIO
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_pullup_en(DHT_PIN);

    // Initialize PostOffice
    post_office_init();

    // Register message handlers
    post_office_register_handler(MESSAGE_LED, handle_led);
    post_office_register_handler(MESSAGE_DHT, handle_dht);
    post_office_register_handler(MESSAGE_BLINK, handle_blink);

    connect_wifi();

    if (wifi_connect_status) {
        ESP_LOGI(TAG, "SPIFFS Web Server is running ... ...\n");
        setup_server();

        mqtt_app_start();
        xTaskCreate(task_publish_dht11, "task_publish_dht11", 4096, 0,5,0);
    }
}