#ifndef ENCODER_H
#define ENCODER_H

#include "project.h"
#include "freertos/portmacro.h"

#define ENCODER_UPDATE_PERIOD_MS 100

#define ENCODER_WHEEL_TWO_PI_R 20.42
#define ENCODER_NUM_DIV 40.0

typedef struct {
    Pin pin;

    volatile int count;
    portMUX_TYPE* mux;
} Encoder;

void encoder_init(Encoder* encoder, int in);
int encoder_getCount(Encoder* encoder);
void encoder_resetCount(Encoder* encoder);

#endif