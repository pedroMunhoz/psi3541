#include "encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

Encoder* encoder_registry[NUM_ENCODERS];

void IRAM_ATTR encoder_ISR_handler(void* arg) {
    int pin = (int)arg;
    for (int i = 0; i < NUM_ENCODERS; i++) {
        if (encoder_registry[i] != NULL && encoder_registry[i]->pin == pin) {
            encoder_registry[i]->count++;
            break;
        }
    }
}

void IRAM_ATTR isr0() { encoder_ISR_handler((void*)&encoder_registry[0]->pin); }
void IRAM_ATTR isr1() { encoder_ISR_handler((void*)&encoder_registry[1]->pin); }
void IRAM_ATTR isr2() { encoder_ISR_handler((void*)&encoder_registry[2]->pin); }
void IRAM_ATTR isr3() { encoder_ISR_handler((void*)&encoder_registry[3]->pin); }

void (*interruptHandlers[NUM_ENCODERS])() = {isr0, isr1, isr2, isr3};

bool encoder_register(Encoder* encoder) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        if (encoder_registry[i] == NULL) {
            encoder_registry[i] = encoder;
            gpio_reset_pin(encoder->pin);
            gpio_set_direction(encoder->pin, GPIO_MODE_INPUT);
            encoder->count = 0;
            gpio_set_intr_type(encoder->pin, GPIO_INTR_POSEDGE); // or GPIO_INTR_ANYEDGE
            gpio_isr_handler_add(encoder->pin, encoder_ISR_handler, (void*)encoder->pin);
            return true;
        }
    }
    return false;
}

void encoder_unregister(Encoder* encoder) {
    for (int i = 0; i < NUM_ENCODERS; i++) {
        if (encoder_registry[i] == encoder) {
            gpio_isr_handler_remove(encoder->pin);
            encoder_registry[i] = NULL;
            return;
        }
    }
}

void encoder_update_task(void* parameter) {
    Encoder* encoder = (Encoder* ) parameter;
    while (true) {
        encoder->vel = 1000.0*(float)encoder->count/ENCODER_UPDATE_PERIOD_MS;
        encoder->count = 0;
        vTaskDelay(pdMS_TO_TICKS(ENCODER_UPDATE_PERIOD_MS));
    }
}

inline int encoder_getPulses(Encoder* encoder) {
    return encoder->count;
}

// In cm/ms
inline float encoder_getSpeed(Encoder* encoder) {
    return (ENCODER_WHEEL_TWO_PI_R/ENCODER_NUM_DIV) * encoder->vel;
}

// In rad/ms
inline float encoder_getAngular(Encoder* encoder) {
    return (6.28/ENCODER_NUM_DIV) * encoder->vel;
}

void encoder_init(Encoder* encoder, int in) {
    static bool isr_installed = false;
    if (!isr_installed) {
        gpio_install_isr_service(0);
        isr_installed = true;
    }
    encoder->pin = in;
    encoder->count = 0;
    encoder->vel = 0.0;
    encoder_register(encoder);

    xTaskCreatePinnedToCore(encoder_update_task,
                            "EncoderUpdateTask",
                            2048,
                            (void*) encoder,
                            4,
                            NULL,
                            1
                        );
}