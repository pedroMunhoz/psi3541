#include "car.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_TASK_STACK_SIZE 4096
#define MOTOR_TASK_PRIORITY   10

static void motor_update_task(void *param) {
    // Motor *motor = (Motor *)param;
    // while (1) {
    //     if (motor->sentido == MOTOR_HORARIO) {
    //         gpio_set_level(motor->in1, 1);
    //         gpio_set_level(motor->in2, 0);
    //     } else if (motor->sentido == MOTOR_ANTIHORARIO) {
    //         gpio_set_level(motor->in1, 0);
    //         gpio_set_level(motor->in2, 1);
    //     } else {
    //         gpio_set_level(motor->in1, 0);
    //         gpio_set_level(motor->in2, 0);
    //     }

    //     if (motor->lado == LEFT) {
    //         mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, motor->pot);
    //         mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    //     } else {
    //         mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, motor->pot);
    //         mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);            
    //     }

    //     vTaskDelay(pdMS_TO_TICKS(10));
    // }
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
    // xTaskCreate(motor_update_task, "motor_update_task", MOTOR_TASK_STACK_SIZE, motor, MOTOR_TASK_PRIORITY, NULL);
}

static motor_sentido_t motor_getDirection(Motor *motor) {
    return motor->sentido;
}

void motor_setDirection(Motor *motor, motor_sentido_t sentido) {
    // if (sentido == MOTOR_HORARIO) {
    //     wifi_debug_printf("\tSetting motor %d in direction - HORARIO\n", motor->lado);
    // } else if (sentido == MOTOR_ANTIHORARIO) {
    //     wifi_debug_printf("\tSetting motor %d in direction - ANTIHORARIO\n", motor->lado);
    // } else {
    //     wifi_debug_printf("\tSetting motor %d parado\n", motor->lado);
    // }

    // wifi_debug_printf("\t\tMotor %d - speed: %.2f - OK\n", motor->lado, encoder_getSpeed(&motor->encoder));

    if (sentido == MOTOR_HORARIO || sentido == MOTOR_ANTIHORARIO || sentido == MOTOR_PARADO) {
        motor->sentido = sentido;
    }

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
}

static int motor_getPot(Motor *motor) {
    return motor->pot;
}

static void motor_setPot(Motor *motor, int pot) {
    if (pot < 70) pot = 70; // Aumente aqui conforme necessário
    if (pot > 100) pot = 100;
    motor->pot = pot;

    if (motor->lado == LEFT) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, motor->pot);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    } else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, motor->pot);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);            
    }
}

void car_move(Car* car, Action action) {
    car->done = false;

    switch (action.state) {
    case STATE_FRENTE:
        car->ref = action.ref * (ENCODER_NUM_DIV / (PI * D_CAR));
        car->state=STATE_FRENTE;
        break;
    case STATE_TRAS:
        car->ref = action.ref * (ENCODER_NUM_DIV / (PI * D_CAR));
        car->state=STATE_TRAS;
        break;
    case STATE_ROT_ESQ:
        car->ref = action.ref * (W_CAR * ENCODER_NUM_DIV) / (360.0 * D_CAR);
        car->state=STATE_ROT_ESQ;
        break;
    case STATE_ROT_DIR:
        car->ref = action.ref * (W_CAR * ENCODER_NUM_DIV) / (360.0 * D_CAR);
        car->state=STATE_ROT_DIR;
        break;
    case STATE_ESQ:
        car->ref = action.ref * (ENCODER_NUM_DIV / (PI * D_CAR));
        car->state=STATE_ESQ;
        break;
    case STATE_DIR:
        car->ref = action.ref * (ENCODER_NUM_DIV / (PI * D_CAR));
        car->state=STATE_DIR;
        break;
    case STATE_STOP:
        car->state=STATE_STOP;
        break;
    default:
        break;
    }
}

static void controlePD(
    int basePotL, int basePotR,
    int deltaL, int deltaR,
    float kp_dir,
    int countL, int countR,
    float kp_dir_total,
    float kd_dir,
    int *potL, int *potR,
    float ki_dir // novo parâmetro Ki
) {
    static int prev_e_dir = 0;
    static float i_e_dir = 0; // termo integrativo

    int e_dir = deltaR - deltaL;
    int delta_e_dir = e_dir - prev_e_dir;
    prev_e_dir = e_dir;
    int e_dir_total = countR - countL;

    i_e_dir += e_dir;
    // Anti-windup: limite o termo integrativo
    if (i_e_dir > 1000) i_e_dir = 1000;
    if (i_e_dir < -1000) i_e_dir = -1000;

    float output_dir = kp_dir * e_dir
                     + kd_dir * delta_e_dir
                     + kp_dir_total * e_dir_total
                     + ki_dir * i_e_dir;

    *potR = basePotR - (int)(output_dir / 2);
    *potL = basePotL + (int)(output_dir / 2);
}

void carControl(void* param) {
    Car* car = (Car*) param;
    static float i_error_dist = 0;
    static int prev_e_dir = 0;

    static int prevCountL = 0;
    static int prevCountR = 0;

    int countL;
    int countR;
    int deltaL;
    int deltaR;

    int potR,potL;

    while(1) {
        switch (car->state) {
        case STATE_FRENTE:
        case STATE_TRAS: {
            bool forward = (car->state == STATE_FRENTE);

            countL = encoder_getCount(&car->motorL.encoder);
            countR = encoder_getCount(&car->motorR.encoder);
            deltaL = countL - prevCountL;
            deltaR = countR - prevCountR;
            prevCountL = countL;
            prevCountR = countR;

            car->cur = (countL + countR) / 2;

            if (car->cur >= car->ref) {
                car->state = STATE_STOP;
                car->cur = 0;
                car->ref = 0;
                i_error_dist = 0;
                prev_e_dir = 0;
                prevCountL = 0;
                prevCountR = 0;
                encoder_resetCount(&car->motorL.encoder);
                encoder_resetCount(&car->motorR.encoder);
                car->done = true;
                break;
            }

            if (car->cur == 0) {
                motor_setDirection(&car->motorR, forward ? MOTOR_HORARIO : MOTOR_ANTIHORARIO);
                motor_setDirection(&car->motorL, forward ? MOTOR_ANTIHORARIO : MOTOR_HORARIO);
                motor_setPot(&car->motorL, 100);
                motor_setPot(&car->motorR, 100);
                vTaskDelay(pdMS_TO_TICKS(250));
            }

            motor_setDirection(&car->motorR, forward ? MOTOR_HORARIO : MOTOR_ANTIHORARIO);
            motor_setDirection(&car->motorL, forward ? MOTOR_ANTIHORARIO : MOTOR_HORARIO);
            
            controlePD(60, 60, 
                                    deltaL, deltaR, car->config.Kp,
                                    countL, countR, car->config.Kp_total,
                                    car->config.Kd, 
                                    &potL, &potR,
                                    car->config.Ki);

            motor_setPot(&car->motorR, potR);
            motor_setPot(&car->motorL, potL);

            wifi_debug_printf("Car %s - %d(goal: %d) | CountL=%d, CountR=%d |PotL=%d, PotR=%d\n",
                forward ? "front" : "back", car->cur, car->ref, countL, countR, potL, potR);
            break;
        }

        case STATE_ROT_ESQ:
        case STATE_ROT_DIR: {
            bool esquerda = (car->state == STATE_ROT_ESQ);

            countL = encoder_getCount(&car->motorL.encoder);
            countR = encoder_getCount(&car->motorR.encoder);
            deltaL = countL - prevCountL;
            deltaR = countR - prevCountR;
            prevCountL = countL;
            prevCountR = countR;

            car->cur = (countL + countR) / 2;

            if (car->cur >= car->ref) {
                car->state = STATE_STOP;
                car->cur = 0;
                car->ref = 0;
                prev_e_dir = 0;
                prevCountL = 0;
                prevCountR = 0;
                encoder_resetCount(&car->motorL.encoder);
                encoder_resetCount(&car->motorR.encoder);
                car->done = true;
                break;
            }

            // --- Controle PD para direção na rotação ---
            int e_dir = deltaR - deltaL; // erro na rotação
            int delta_e_dir = e_dir - prev_e_dir;
            prev_e_dir = e_dir;
            float output_dir = KP_DIR * e_dir + KD_DIR * delta_e_dir;

            // Ajusta potência dos motores para corrigir desvio na rotação
            int potR = motor_getPot(&car->motorR) - (int)(output_dir / 2);
            int potL = motor_getPot(&car->motorL) + (int)(output_dir / 2);

            motor_setDirection(&car->motorR, esquerda ? MOTOR_HORARIO : MOTOR_ANTIHORARIO);
            motor_setDirection(&car->motorL, esquerda ? MOTOR_HORARIO : MOTOR_ANTIHORARIO);

            controlePD(motor_getPot(&car->motorL), motor_getPot(&car->motorR), 
                                    deltaL, deltaR, car->config.Kp,
                                    0, 0, 0,
                                    car->config.Kd, 
                                    &potL, &potR,
                                    car->config.Ki);

            motor_setPot(&car->motorR, potR);
            motor_setPot(&car->motorL, potL);

            wifi_debug_printf("Car rotated %s - %d(goal: %d) | PotL=%d, PotR=%d\n",
                esquerda ? "left" : "right", car->cur, car->ref, potL, potR);
            break;
        }

        case STATE_STOP:          
            motor_setDirection(&car->motorR, MOTOR_PARADO);
            motor_setDirection(&car->motorL, MOTOR_PARADO);
            break;
        
        default:
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void handleAction(messenger_message_t *message, void *context) {
    Car* car = (Car*) context;

    Action action;
    if (!message || !message->data) {
        wifi_debug_printf("[handleAction] ERRO: message ou message->data nulo!\n");
        int resp = 0;
        messenger_setResponse(message, (void*)&resp);
        return;
    }
    messenger_getStruct_from_message_data(message, &action, sizeof(Action));

    wifi_debug_printf("[handleAction] Chamou acao com - state: %d, ref: %d\n", action.state, action.ref);
    
    car_move(car, action);

    int resp = 1;
    messenger_setResponse(message, (void*)&resp);
}

static void handleSetConfig(messenger_message_t *message, void *context) {
    Car* car = (Car*) context;

    if (!message || !message->data) {
        wifi_debug_printf("[handleSetConfig] ERRO: message ou message->data nulo!\n");
        return;
    }
    messenger_getStruct_from_message_data(message, &car->config, sizeof(CarConfig));

    wifi_debug_printf("[handleSetConfig] Configuração recebida: Kp=%.3f, Kp_total=%.3f, Kd=%.3f\n",
        car->config.Kp, car->config.Kp_total, car->config.Kd);

    messenger_setResponse(message, (void*)&car->config);
}

static void handleGetConfig(messenger_message_t *message, void *context) {
    Car* car = (Car*) context;
    wifi_debug_printf("[handleGetConfig] Enviando configuração atual: Kp=%.3f, Kp_total=%.3f, Kd=%.3f\n",
        car->config.Kp, car->config.Kp_total, car->config.Kd);

    messenger_setResponse(message, (void*)&car->config);
}

static void handleGetStatus(messenger_message_t* message, void* context) {
    Car* car = (Car*) context;

    CarStatus status = {.cur = car->cur, .done = car->done, .ref=car->ref, .state = car->state};
    wifi_debug_printf("[handleGetStatus] Status: state=%d, ref=%d, cur=%d, done=%d\n",
        status.state, status.ref, status.cur, status.done);

    messenger_setResponse(message, (void*)&status);
}


void car_init(Car* car, Pin in1, Pin in2, Pin in3, Pin in4, Pin enA, Pin enB) {
    motor_init(&(car->motorR), in1, in2, enA, RIGHT);
    motor_init(&(car->motorL), in3, in4, enB, LEFT);

    car->cur = 0;
    car->done = false;
    car->ref = 0;
    car->state = STATE_STOP;
    car->config.Kp = KP_DIR;
    car->config.Kp_total = KP_DIR_TOTAL;
    car->config.Kd = KD_DIR;
    car->config.Ki = KI_DIR;

    xTaskCreate(carControl, "car_control_task", 4096, car, 20, NULL);
}

void car_setMessenger(Car* car, Messenger* messenger) {
    car->messenger = messenger;

    messenger_register_handler(car->messenger, MESSAGE_CAR_MOVE, handleAction, car);
    messenger_register_handler(car->messenger, MESSAGE_CAR_GET_CONFIG, handleGetConfig, car);
    messenger_register_handler(car->messenger, MESSAGE_CAR_SET_CONFIG, handleSetConfig, car);
    messenger_register_handler(car->messenger, MESSAGE_CAR_GET_STATUS, handleGetStatus, car);
}