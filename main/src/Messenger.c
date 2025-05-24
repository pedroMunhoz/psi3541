#include "Messenger.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define MAX_MESSAGE_HANDLERS 10 // Maximum number of message types

static const char *TAG = "Messenger";

// Define the queue
static QueueHandle_t messenger_queue;

// Define the handler registry
typedef struct {
    messenger_message_type_t type;
    messenger_callback_t callback;
} messenger_handler_t;

static messenger_handler_t handlers[MAX_MESSAGE_HANDLERS];
static int handler_count = 0;

static void messenger_task(void *param) {
    messenger_message_t message;

    while (1) {
        // Wait for a message
        if (xQueueReceive(messenger_queue, &message, portMAX_DELAY)) {
            // Find the appropriate handler
            for (int i = 0; i < handler_count; i++) {
                if (handlers[i].type == message.type) {
                    if (handlers[i].callback) {
                        handlers[i].callback(&message); // Pass the entire message
                    }
                    break;
                }
            }
        }
    }
}

void messenger_init(void) {
    // Create the queue
    messenger_queue = xQueueCreate(10, sizeof(messenger_message_t));
    if (messenger_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create Messenger queue");
        return;
    }

    // Create the Messenger task
    xTaskCreate(messenger_task, "MessengerTask", 2048, NULL, 5, NULL);
}

// Send a message to the Messenger
void messenger_send_message(messenger_message_t *message) {
    if (xQueueSend(messenger_queue, message, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send message to Messenger");
    }
}

// Register a handler for a specific message type
void messenger_register_handler(messenger_message_type_t type, messenger_callback_t callback) {
    if (handler_count < MAX_MESSAGE_HANDLERS) {
        handlers[handler_count].type = type;
        handlers[handler_count].callback = callback;
        handler_count++;
        ESP_LOGI(TAG, "Handler registered for message type %d", type);
    } else {
        ESP_LOGE(TAG, "Handler registry is full");
    }
}

void messenger_setResponse(messenger_message_t *message, void *data) {
    if (message->response_queue != NULL) {
        if (xQueueSend(message->response_queue, data, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "Failed to send response to the response queue");
        } else {
            ESP_LOGI(TAG, "Response sent successfully");
        }
    } else {
        ESP_LOGW(TAG, "No response queue available for this message");
    }
}