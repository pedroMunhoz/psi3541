#ifndef POST_OFFICE_H
#define POST_OFFICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "project.h"

typedef struct {
    post_office_message_type_t type;
    void *data;
    QueueHandle_t response_queue;
} post_office_message_t;

typedef void (*post_office_callback_t)(post_office_message_t *message);

void post_office_init(void);
void post_office_send_message(post_office_message_t *message);
void post_office_register_handler(post_office_message_type_t type, post_office_callback_t callback);
void setResponse(post_office_message_t *message, void *data);

#endif // POST_OFFICE_H