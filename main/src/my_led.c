#include "my_led.h"

static const char *TAG = "MyLED"; // TAG for debug

int blink_freq = 1;

/* ###################### MODULE TASKS ######################*/
static void blink_task(void *pvParameter) {
    while (1) {
        int current_level = gpio_get_level(LED_PIN);
        gpio_set_level(LED_PIN, !current_level);
        vTaskDelay(pdMS_TO_TICKS(1000 / blink_freq));
    }
}

/* ###################### MODULE HANDLERS ######################*/
static void handle_led(messenger_message_t *message) {
    if (message->data != NULL) {
        int *state = (int *)message->data;
        gpio_set_level(LED_PIN, *state);
        ESP_LOGI("LED", "LED set to: %d", *state);
    } 
    int led_state = gpio_get_level(LED_PIN);
    ESP_LOGI("LED", "LED state: %d", led_state);

    messenger_setResponse(message, &led_state);
}

static void handle_blink(messenger_message_t *message) {
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
    messenger_setResponse(message, &blink_state);
}

/* ###################### MODULE EXTERNAL FUNCTIONS ######################*/
void init_led() {
    // Initialize GPIO
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_INPUT_OUTPUT);

    messenger_register_handler(MESSAGE_LED, handle_led);
    messenger_register_handler(MESSAGE_BLINK, handle_blink);
}