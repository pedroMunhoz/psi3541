#include "car.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
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

        if (motor->lado == LEFT) {
            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, motor->pot);
            mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
        } else {
            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, motor->pot);
            mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);            
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Update every 20ms
    }
}

static void motor_init(Motor* motor, Pin in1, Pin in2, Pin en, Lado lado) {
    motor->in1 = in1;
    motor->in2 = in2;
    motor->en = en;
    motor->lado = lado;

    motor->pot = 80;
    motor->sentido = MOTOR_PARADO;

    gpio_reset_pin(motor->in1);
    gpio_set_direction(motor->in1, GPIO_MODE_OUTPUT);
    gpio_reset_pin(motor->in2);
    gpio_set_direction(motor->in2, GPIO_MODE_OUTPUT);
    gpio_reset_pin(motor->en);
    gpio_set_direction(motor->en, GPIO_MODE_OUTPUT);

    if(lado == LEFT) {
	    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, motor->en);
    } else { 
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, motor->en);
    }
	mcpwm_config_t pwm_config = {
		.frequency = PWM_FREQ,
		.cmpr_a = 0,
		.duty_mode = MCPWM_DUTY_MODE_0,
		.counter_mode = MCPWM_UP_COUNTER
	};
	mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    // Create the update task for this motor
    xTaskCreate(motor_update_task, "motor_update_task", MOTOR_TASK_STACK_SIZE, motor, MOTOR_TASK_PRIORITY, NULL);
}

static motor_sentido_t motor_getDirection(Motor *motor) {
    return motor->sentido;
}

void motor_setDirection(Motor *motor, motor_sentido_t sentido) {
    if (sentido == MOTOR_HORARIO) {
        wifi_debug_printf("\tSetting motor %d in direction - HORARIO\n", motor->lado);
    } else if (sentido == MOTOR_ANTIHORARIO) {
        wifi_debug_printf("\tSetting motor %d in direction - ANTIHORARIO\n", motor->lado);
    } else {
        wifi_debug_printf("\tSetting motor %d parado\n", motor->lado);
    }

    wifi_debug_printf("\t\tMotor %d - speed: %.2f - OK\n", motor->lado, encoder_getSpeed(&motor->encoder));

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

    wifi_debug_printf("\tSetting pot of motor %d to %d%%\n", motor->lado, pot);
    motor->pot = pot;
}

void car_move(Car* car, Action acao) {

    switch (acao.move) {
    case FRENTE:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, acao.pot);
        motor_setPot(&car->motorL, acao.pot);
        break;
    case TRAS:
        motor_setDirection(&car->motorR, MOTOR_ANTIHORARIO);
        motor_setDirection(&car->motorL, MOTOR_HORARIO);
        motor_setPot(&car->motorR, acao.pot);
        motor_setPot(&car->motorL, acao.pot);
        break;
    case ROT_ESQUERDA:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_HORARIO);
        motor_setPot(&car->motorR, acao.pot);
        motor_setPot(&car->motorL, acao.pot);
        break;
    case ROT_DIREITA:
        motor_setDirection(&car->motorR, MOTOR_ANTIHORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, acao.pot);
        motor_setPot(&car->motorL, acao.pot);
        break;
    case ESQUERDA:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, acao.pot);
        motor_setPot(&car->motorL, acao.pot*75/100);
        break;
    case DIREITA:
        motor_setDirection(&car->motorR, MOTOR_HORARIO);
        motor_setDirection(&car->motorL, MOTOR_ANTIHORARIO);
        motor_setPot(&car->motorR, acao.pot*75/100);
        motor_setPot(&car->motorL, acao.pot);
        break;
    case PARAR:
        motor_setDirection(&car->motorR, MOTOR_PARADO);
        motor_setDirection(&car->motorL, MOTOR_PARADO);
        motor_setPot(&car->motorR, acao.pot);
        motor_setPot(&car->motorL, acao.pot);
        break;
    default:
        break;
    }
}

static void handleAction(messenger_message_t *message, void *context) {
    Car* car = (Car*) context;

    Action action;
    messenger_getStruct_from_message_data(message, &action, sizeof(Action));

    wifi_debug_printf("Chamou acao com - move: %d | pot: %d\n", action.move, action.pot);

    car_move(car, action);
}

void car_init(Car* car, Pin in1, Pin in2, Pin in3, Pin in4, Pin enA, Pin enB) {
    motor_init(&(car->motorR), in1, in2, enA, RIGHT);
    motor_init(&(car->motorL), in3, in4, enB, LEFT);
}

void car_setMessenger(Car* car, Messenger* messenger) {
    car->messenger = messenger;

    messenger_register_handler(car->messenger, MESSAGE_CAR_MOVE, handleAction, car);
}