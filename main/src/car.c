#include "car.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_TASK_STACK_SIZE 4096
#define MOTOR_TASK_PRIORITY   10

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

        ledc_set_duty(LEDC_LOW_SPEED_MODE, motor->en, motor->pot);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, motor->en);

        vTaskDelay(pdMS_TO_TICKS(10)); // Update every 20ms
    }
}

static int motorChannel = 0;

static void motor_init(Motor* motor, Pin in1, Pin in2, Pin en) {
    motor->in1 = in1;
    motor->in2 = in2;
    motor->en = en;

    motor->pot = 80;
    motor->sentido = MOTOR_PARADO;

    gpio_reset_pin(motor->in1);
    gpio_set_direction(motor->in1, GPIO_MODE_OUTPUT);
    gpio_reset_pin(motor->in2);
    gpio_set_direction(motor->in2, GPIO_MODE_OUTPUT);
    gpio_reset_pin(motor->en);
    gpio_set_direction(motor->en, GPIO_MODE_OUTPUT);

    static bool ledc_initialized = false;
    if (!ledc_initialized) {
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_LOW_SPEED_MODE,
            .timer_num        = LEDC_TIMER_0,
            .duty_resolution  = LEDC_TIMER_8_BIT,
            .freq_hz          = 5000,
            .clk_cfg          = LEDC_AUTO_CLK
        };
        ledc_timer_config(&ledc_timer);
        ledc_initialized = true;
    }

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = motor->en,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = (motorChannel == 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1),
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0
    };
    motorChannel++;
    ledc_channel_config(&ledc_channel);

    // Create the update task for this motor
    xTaskCreate(motor_update_task, "motor_update_task", MOTOR_TASK_STACK_SIZE, motor, MOTOR_TASK_PRIORITY, NULL);
}

static motor_sentido_t motor_getDirection(Motor *motor) {
    return motor->sentido;
}

void motor_setDirection(Motor *motor, motor_sentido_t sentido) {
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

void car_move(Car* car, Move acao) {
    switch (acao) {
    case FRENTE:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, 100);
        motor_setPot(&car->motorL, 100);
        break;
    case TRAS:
        motor_setDirection(&car->motorR, MOTOR_ANTIHORARIO);
        motor_setDirection(&car->motorL, MOTOR_HORARIO);
        motor_setPot(&car->motorR, 100);
        motor_setPot(&car->motorL, 100);
        break;
    case ROT_ESQUERDA:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_HORARIO);
        motor_setPot(&car->motorR, 100);
        motor_setPot(&car->motorL, 100);
        break;
    case ROT_DIREITA:
        motor_setDirection(&car->motorR, MOTOR_ANTIHORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, 100);
        motor_setPot(&car->motorL, 100);
        break;
    case ESQUERDA:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, 100);
        motor_setPot(&car->motorL, 80);
        break;
    case DIREITA:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, 80);
        motor_setPot(&car->motorL, 100);
        break;
    case PARAR:
        motor_setDirection(&car->motorR, MOTOR_PARADO);
        motor_setDirection(&car->motorL, MOTOR_PARADO);
        motor_setPot(&car->motorR, 100);
        motor_setPot(&car->motorL, 100);
        break;
    default:
        break;
    }
}

static void handleAction(messenger_message_t *message, void *context) {
    Car* car = (Car*) context;

    Move action = message->int_data;

    car_move(car, action);
}

void car_init(Car* car, Pin in1, Pin in2, Pin in3, Pin in4, Pin enA, Pin enB) {
    motor_init(&(car->motorR), in1, in2, enA);
    motor_init(&(car->motorL), in3, in4, enB);
}

void car_setMessenger(Car* car, Messenger* messenger) {
    car->messenger = messenger;

    messenger_register_handler(car->messenger, MESSAGE_CAR_MOVE, handleAction, car);
}