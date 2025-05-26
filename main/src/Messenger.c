#include "Messenger.h"

#define MESSENGER_MSG_TIMEOUT pdMS_TO_TICKS(2000)     // 2 seconds for message receive
#define MESSENGER_RESP_TIMEOUT pdMS_TO_TICKS(2000)    // 2 seconds for response receive

static const char *TAG = "Messenger";

static void messenger_task(void *param) {
    Messenger* messenger = (Messenger*) param;
    messenger_message_t message;

    while (1) {
        if (xQueueReceive(messenger->msg_queue, &message, MESSENGER_MSG_TIMEOUT)) {
            bool handled = false;
            for (int i = 0; i < messenger->handler_count; i++) {
                if (messenger->handlers[i].type == message.type) {
                    if (messenger->handlers[i].callback) {
                        messenger->handlers[i].callback(&message, messenger->handlers[i].context);
                        handled = true;
                    }
                    break;
                }
            }
            if (!handled) {
                ESP_LOGW(TAG, "No handler for message type %d", message.type);
            }
        }
        // else: Timeout, loop again (can add a vTaskDelay if desired)
    }
}

void messenger_init(Messenger* messenger) {
    ESP_LOGI(TAG, "Initializing Messenger...");
    messenger->handler_count = 0;

    messenger->msg_queue = xQueueCreate(MESSENGER_QUEUE_SIZE, sizeof(messenger_message_t));
    if (messenger->msg_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create Messenger queue");
        return;
    }

    xTaskCreate(messenger_task, "MessengerTask", 2048, messenger, 5, NULL);
    ESP_LOGI(TAG, "Messenger initialized.");
}

void messenger_send_message(Messenger* messenger, messenger_message_t *message) {
    if (xQueueSend(messenger->msg_queue, message, MESSENGER_MSG_TIMEOUT) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send message to Messenger (timeout)");
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
        if (xQueueSend(message->response_queue, data, MESSENGER_RESP_TIMEOUT) != pdPASS) {
            ESP_LOGE(TAG, "Failed to send response to the response queue (timeout)");
        }
    }
}

void messenger_send_generic(Messenger *messenger, messenger_message_type_t type, void *data) {

    messenger_message_t message = {
        .type = type,
        .data = data,
        .response_queue = NULL
    };

    messenger_send_message(messenger, &message);
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

    bool success = xQueueReceive(response_queue, response_out, MESSENGER_RESP_TIMEOUT);
    if (!success) {
        ESP_LOGE(TAG, "Timeout waiting for response to message type %d", type);
    }
    vQueueDelete(response_queue);
    return success;
}
