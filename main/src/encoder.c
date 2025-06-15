#include "encoder.h"

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
            gpio_set_intr_type(encoder->pin, GPIO_INTR_ANYEDGE);
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

int encoder_getCount(Encoder* encoder) {
    portENTER_CRITICAL(encoder->mux);
    int count = encoder->count;
    portEXIT_CRITICAL(encoder->mux);
    return count;
}

void encoder_resetCount(Encoder* encoder) {
    portENTER_CRITICAL(encoder->mux);
    encoder->count = 0;
    portEXIT_CRITICAL(encoder->mux);
}

void encoder_init(Encoder* encoder, int in) {
    static bool isr_installed = false;
    if (!isr_installed) {
        gpio_install_isr_service(0);
        isr_installed = true;
    }
    encoder->pin = in;
    encoder->count = 0; 
    encoder->mux = malloc(sizeof(portMUX_TYPE));
    if (encoder->mux != NULL) {
        *encoder->mux = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;
    }
    encoder_register(encoder);
}