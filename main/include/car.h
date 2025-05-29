#ifndef CAR_H
#define CAR_H

#include "project.h"
#include "Messenger.h"

typedef enum {
    MOTOR_HORARIO,
    MOTOR_ANTIHORARIO,
    MOTOR_PARADO
} motor_sentido_t;

typedef struct {
    Pin in1;
    Pin in2;
    Pin en;

    motor_sentido_t sentido;
    int pot;
} Motor;

typedef struct {
    int x;
    int y;
    int z;
} Vec3_int_t;

typedef struct {
    Vec3_int_t acc;
    Vec3_int_t gyr;
} Mpu;

typedef struct {
    Motor motorR;
    Motor motorL;

    Messenger* messenger;
} Car;

#endif