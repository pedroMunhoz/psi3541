#ifndef MYLED_H
#define MYLED_H

#include <driver/gpio.h>
#include "Messenger.h"

typedef enum {
    NORMAL,
    BLINKING
} LedMode;

typedef enum {
    LED_CHECK_CHANGE_ON,
    LED_CHECK_CHANGE_OFF
} LedCheckMode;

typedef struct {
    int pin;

    int blink_freq;
    int state;
    int lastState;

    LedMode blink_state;

    Messenger* messenger;
} myLED;

void led_init(myLED* led, int pin);
void led_setMessenger(myLED* led, Messenger* messenger);
void led_check_change(myLED* led, int mode);

#endif