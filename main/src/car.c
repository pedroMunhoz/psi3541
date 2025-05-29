#include "car.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_TASK_STACK_SIZE 2048
#define MOTOR_TASK_PRIORITY   5

static void motor_update_task(void *param) {
    Motor *motor = (Motor *)param;
    while (1) {
        if (motor->sentido == MOTOR_HORARIO) {
            gpio_set_level(motor->in1, 1);
            gpio_set_level(motor->in2, 0);
        } else if (motor->sentido == MOTOR_ANTIHORARIO) {
            gpio_set_level(motor->in1, 0);
            gpio_set_level(motor->in2, 1);
        } else {
            gpio_set_level(motor->in1, 0);
            gpio_set_level(motor->in2, 0);
        }
        // Set PWM value to motor power (pot: 0-100)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->pwm_channel, motor->pot); // Assuming pwm_channel is set in Motor struct
        ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->pwm_channel);

        vTaskDelay(pdMS_TO_TICKS(20)); // Update every 20ms
    }
}

static void motor_init(Motor* motor, Pin in1, Pin in2, Pin en) {
    motor->in1 = in1;
    motor->in2 = in2;
    motor->en = en;

    motor->pot = 70;
    motor->sentido = MOTOR_PARADO;

    gpio_reset_pin(motor->in1);
    gpio_set_direction(motor->in1, GPIO_MODE_OUTPUT);
    gpio_reset_pin(motor->in2);
    gpio_set_direction(motor->in2, GPIO_MODE_OUTPUT);
    gpio_reset_pin(motor->en);
    gpio_set_direction(motor->en, GPIO_MODE_OUTPUT);

    // Create the update task for this motor
    xTaskCreate(motor_update_task, "motor_update_task", MOTOR_TASK_STACK_SIZE, motor, MOTOR_TASK_PRIORITY, NULL);
}

static motor_sentido_t motor_getDirection(Motor *motor) {
    return motor->sentido;
}

static void motor_setDirection(Motor *motor, motor_sentido_t sentido) {
    if (sentido == MOTOR_HORARIO || sentido == MOTOR_ANTIHORARIO || sentido == MOTOR_PARADO) {
        motor->sentido = sentido;
    }
}

static int motor_getPot(Motor *motor) {
    return motor->pot;
}

static void motor_setPot(Motor *motor, int pot) {
    if (pot < 0) pot = 0;
    if (pot > 100) pot = 100;
    motor->pot = pot;
}

car_init(Car* car, in1, in2, in3, in4, enA, enB) {
    motor_init(&(car->motorR), in1, in2, enA);
    motor_init(&(car->motorL), in3, in4, enB);
}

car_setMessenger(Car* car, Messenger* messenger) {
    car->messenger = messenger;
}