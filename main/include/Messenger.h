#ifndef MESSENGER_H
#define MESSENGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "project.h"

#define MESSENGER_QUEUE_SIZE 10
#define MAX_MESSAGE_HANDLERS 15

typedef struct {
    messenger_message_type_t type;
    void *data;
    QueueHandle_t response_queue;
} messenger_message_t;

typedef void (*messenger_callback_t)(messenger_message_t *message, void *context);

typedef struct {
    messenger_message_type_t type;
    messenger_callback_t callback;
    void *context;
} messenger_handler_t;

typedef struct {
    QueueHandle_t msg_queue;
    messenger_handler_t handlers[MAX_MESSAGE_HANDLERS];
    int handler_count;
} Messenger;

void messenger_init(Messenger* messenger);
void messenger_send_message(Messenger* messenger, messenger_message_t *message);
void messenger_register_handler(Messenger* messenger, messenger_message_type_t type, messenger_callback_t callback, void *context);
void messenger_setResponse(messenger_message_t *message, void *data);
bool messenger_send_with_response_generic(Messenger *messenger, messenger_message_type_t type, void *data, void *response_out, size_t response_size);
void messenger_send_generic(Messenger *messenger, messenger_message_type_t type, void *data);

#endif // MESSENGER_H