#include <stdio.h>
#include <stdlib.h>
#include <string.h> //Requires by memset

#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "project.h"
#include "connect_wifi.h"
#include "PostOffice.h"
#include "server.h"

static const char *TAG = "espressif"; // TAG for debug

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

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize PostOffice
    post_office_init();

    // Register message handlers
    post_office_register_handler(MESSAGE_LED, handle_led);

    connect_wifi();

    if (wifi_connect_status) {
        gpio_reset_pin(LED_PIN);
        gpio_set_direction(LED_PIN, GPIO_MODE_INPUT_OUTPUT);

        ESP_LOGI(TAG, "SPIFFS Web Server is running ... ...\n");
        setup_server();
    }
}