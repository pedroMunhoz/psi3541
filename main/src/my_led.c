#include "my_led.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "MyLED"; // TAG for debug

/* ###################### MODULE TASKS ######################*/
static void led_task(void* pvParameter) {
    myLED* led = (myLED*) pvParameter;

    while(1) {
        led->lastState = led->state;
        led->state = gpio_get_level(led->pin);

        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD));
    }
}

static void led_checkChange_task(void* pvParameter) {
    myLED* led = (myLED*) pvParameter;
    messenger_message_t message = {.type = MESSAGE_LED_CHANGED, .data = NULL, .response_queue = NULL};

    while(1) {
        if (led->state != led->lastState) {
            message.data = (int*) &(led->state);
            messenger_send_message(led->messenger, &message);
        }
        vTaskDelay(pdMS_TO_TICKS(LED_UPDATE_PERIOD));
    }
}

static void blink_task(void *pvParameter) {
    myLED* led = (myLED*) pvParameter;

    while (1) {
        int current_level = led->state;
        gpio_set_level(led->pin, !current_level);
        vTaskDelay(pdMS_TO_TICKS(1000 / led->blink_freq));
    }
}

/* ###################### MODULE HANDLERS ######################*/
static void handle_led(messenger_message_t *message, void* context) {
    myLED* led = (myLED*) context;

    if (message->data != NULL) {
        int *state = (int *)message->data;
        gpio_set_level(led->pin, *state);

    }
    led->lastState = led->state;
    led->state = gpio_get_level(led->pin);

    messenger_setResponse(message, &(led->state));
}

static void handle_blink(messenger_message_t *message, void* context) {
    myLED* led = (myLED*) context;

    if (message->data != NULL) {
        int *frequency = (int *)message->data;
        led->blink_freq = *frequency;
    } else {
        if (xTaskGetHandle("blink_task") != NULL) {
            vTaskDelete(xTaskGetHandle("blink_task"));
            led->blink_state = NORMAL;
        } else {
            xTaskCreate(blink_task, "blink_task", 2048, led, 5, NULL);
            led->blink_state = BLINKING;
        }
    }
    messenger_setResponse(message, &(led->blink_state));
}

/* ###################### MODULE EXTERNAL FUNCTIONS ######################*/
void led_init(myLED* led, int pin) {
    led->blink_freq=1;
    led->pin=pin;
    led->state = 0;
    led->lastState = led->state;
    led->blink_state = NORMAL;

    gpio_reset_pin(led->pin);
    gpio_set_direction(led->pin, GPIO_MODE_INPUT_OUTPUT);

    xTaskCreate(led_task, "led_task", 2048, led, 5, NULL);
}

void led_setMessenger(myLED* led, Messenger* messenger) {
    led->messenger = messenger;

    messenger_register_handler(led->messenger, MESSAGE_LED, handle_led, led);
    messenger_register_handler(led->messenger, MESSAGE_BLINK, handle_blink, led);
}

void led_check_change(myLED* led, int mode) {
    if (mode == LED_CHECK_CHANGE_ON) {
        xTaskCreate(led_checkChange_task, "led_checkChange_task", 2048, led, 5, NULL);
    } else if (mode == LED_CHECK_CHANGE_OFF) {
        if (xTaskGetHandle("led_checkChange_task") != NULL) {
            vTaskDelete(xTaskGetHandle("led_checkChange_task"));
        }
    }
}