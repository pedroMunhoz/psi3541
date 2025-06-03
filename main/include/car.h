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

typedef enum {
    FRENTE,
    TRAS,
    ESQUERDA,
    DIREITA,
    ROT_ESQUERDA,
    ROT_DIREITA,
    PARAR
} Move;

typedef struct {
    Motor motorR;
    Motor motorL;

    Messenger* messenger;
} Car;

void car_init(Car* car, Pin in1, Pin in2, Pin in3, Pin in4, Pin enA, Pin enB);
void car_setMessenger(Car* car, Messenger* messenger);
void car_move(Car* car, Move acao);
void motor_setDirection(Motor *motor, motor_sentido_t sentido);

#endif