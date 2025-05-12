#ifndef POST_OFFICE_H
#define POST_OFFICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Define message types
typedef enum {
    MESSAGE_LED,
    MESSAGE_UNKNOWN
} post_office_message_type_t;

// Define the message structure
typedef struct {
    post_office_message_type_t type;
    void *data; // Optional data for the message
    QueueHandle_t response_queue; // Queue for sending a response
} post_office_message_t;

typedef void (*post_office_callback_t)(post_office_message_t *message);

// Function prototypes
void post_office_init(void);
void post_office_send_message(post_office_message_t *message);
void post_office_register_handler(post_office_message_type_t type, post_office_callback_t callback);
void setResponse(post_office_message_t *message, void *data);

#endif // POST_OFFICE_H