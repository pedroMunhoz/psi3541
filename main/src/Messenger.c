#include "Messenger.h"

static const char *TAG = "Messenger";

static void messenger_task(void *param) {
    Messenger* messenger = (Messenger*) param;
    messenger_message_t message;

    while (1) {
        if (xQueueReceive(messenger->msg_queue, &message, portMAX_DELAY)) {
            for (int i = 0; i < messenger->handler_count; i++) {
                if (messenger->handlers[i].type == message.type) {
                    if (messenger->handlers[i].callback) {
                        messenger->handlers[i].callback(&message, messenger->handlers[i].context);
                    }
                    break;
                }
            }
        }
    }
}

void messenger_init(Messenger* messenger) {
    messenger->handler_count = 0;

    messenger->msg_queue = xQueueCreate(MESSENGER_QUEUE_SIZE, sizeof(messenger_message_t));
    if (messenger->msg_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create Messenger queue");
        return;
    }

    xTaskCreate(messenger_task, "MessengerTask", 2048, messenger, 5, NULL);
}

void messenger_send_message(Messenger* messenger, messenger_message_t *message) {
    if (xQueueSend(messenger->msg_queue, message, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send message to Messenger");
    }
}

void messenger_register_handler(Messenger* messenger, messenger_message_type_t type, messenger_callback_t callback, void *context) {
    if (messenger->handler_count < MAX_MESSAGE_HANDLERS) {
        messenger->handlers[messenger->handler_count].type = type;
        messenger->handlers[messenger->handler_count].callback = callback;
        messenger->handlers[messenger->handler_count].context = context;
        messenger->handler_count++;
    } else {
        ESP_LOGE(TAG, "Handler registry is full");
    }
}

void messenger_setResponse(messenger_message_t *message, void *data) {
    if (message->response_queue != NULL) {
        if (xQueueSend(message->response_queue, data, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "Failed to send response to the response queue");
        }
    }
}

bool messenger_send_with_response_generic(Messenger *messenger, messenger_message_type_t type, void *data, void *response_out, size_t response_size) {
    QueueHandle_t response_queue = xQueueCreate(1, response_size);
    if (!response_queue) {
        ESP_LOGE(TAG, "Failed to create response queue");
        return false;
    }

    messenger_message_t message = {
        .type = type,
        .data = data,
        .response_queue = response_queue
    };

    messenger_send_message(messenger, &message);

    bool success = xQueueReceive(response_queue, response_out, portMAX_DELAY);
    vQueueDelete(response_queue);
    return success;
}
