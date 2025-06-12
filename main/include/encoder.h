#ifndef ENCODER_H
#define ENCODER_H

#include "project.h"

#define ENCODER_UPDATE_PERIOD_MS 500

#define ENCODER_WHEEL_TWO_PI_R 20.42
#define ENCODER_NUM_DIV 20.0

typedef struct {
    Pin pin;

    volatile int count;
    float vel;
} Encoder;

void encoder_init(Encoder* encoder, int in);
int encoder_getPulses(Encoder* encoder);
float encoder_getSpeed(Encoder* encoder);
float encoder_getAngular(Encoder* encoder);

#endif