#ifndef MESSENGER_H
#define MESSENGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "project.h"

typedef struct {
    messenger_message_type_t type;
    void *data;
    QueueHandle_t response_queue;
} messenger_message_t;

typedef void (*messenger_callback_t)(messenger_message_t *message);

void messenger_init(void);
void messenger_send_message(messenger_message_t *message);
void messenger_register_handler(messenger_message_type_t type, messenger_callback_t callback);
void messenger_setResponse(messenger_message_t *message, void *data);

#endif // MESSENGER_H