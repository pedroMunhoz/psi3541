#include "PostOffice.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define MAX_MESSAGE_HANDLERS 10 // Maximum number of message types

static const char *TAG = "PostOffice";

// Define the queue
static QueueHandle_t post_office_queue;

// Define the handler registry
typedef struct {
    post_office_message_type_t type;
    post_office_callback_t callback;
} post_office_handler_t;

static post_office_handler_t handlers[MAX_MESSAGE_HANDLERS];
static int handler_count = 0;

static void post_office_task(void *param) {
    post_office_message_t message;

    while (1) {
        // Wait for a message
        if (xQueueReceive(post_office_queue, &message, portMAX_DELAY)) {
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

// Initialize the PostOffice
void post_office_init(void) {
    // Create the queue
    post_office_queue = xQueueCreate(10, sizeof(post_office_message_t));
    if (post_office_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create PostOffice queue");
        return;
    }

    // Create the PostOffice task
    xTaskCreate(post_office_task, "PostOfficeTask", 2048, NULL, 5, NULL);
}

// Send a message to the PostOffice
void post_office_send_message(post_office_message_t *message) {
    if (xQueueSend(post_office_queue, message, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send message to PostOffice");
    }
}

// Register a handler for a specific message type
void post_office_register_handler(post_office_message_type_t type, post_office_callback_t callback) {
    if (handler_count < MAX_MESSAGE_HANDLERS) {
        handlers[handler_count].type = type;
        handlers[handler_count].callback = callback;
        handler_count++;
        ESP_LOGI(TAG, "Handler registered for message type %d", type);
    } else {
        ESP_LOGE(TAG, "Handler registry is full");
    }
}

void setResponse(post_office_message_t *message, void *data) {
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